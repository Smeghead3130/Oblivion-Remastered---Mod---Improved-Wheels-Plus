#include "PerfDiag.h"
#ifdef IWP_TEMP_TIMINGS
#include "PEHook.h"
#include <array>
#include <algorithm>
#include <atomic>
#include <cmath>

// TEMP-DIAG: fixed-size per-thread histograms, bounded 180-second capture.
// Inclusive timings include nested instrument overhead. No per-call logging.
namespace perfdiag
{
	namespace
	{
		constexpr std::size_t kMetrics = static_cast<std::size_t>(Metric::Count);
		constexpr std::size_t kCounters = static_cast<std::size_t>(Counter::Count);
		constexpr const char* kNames[] = {
			"MagicTick.other", "MagicTick.magic", "MagicTick.ammo", "MagicTick.inventoryPanel", "MagicTick.magicPanel",
			"GameImage", "VisibleWheel", "ShownOnScreen", "Measure", "EnsureLoaded", "MagicSlots", "HighlightedItem",
			"GameMessageUp.classMissing", "GameMessageUp.classFound", "Chained", "previous.user0", "previous.user1", "previous.user2", "previous.user3",
			"Keyboard.Apply", "Rewrite", "frameCallback", "Controls.Tick", "UnbindNativeQuickKeys", "Follow", "Harvest.full", "FindSaveData",
			"Instances", "Save", "RemoveGoneInMenus", "SlotCounts", "Keys", "Offset.RenderOpacity", "GetAsyncKeyState", "ui.Load",
			"BoostAnimations", "RingOverlay.Show", "FindInTree", "SessionContext.Current", "TES.pass"
		};
		static_assert(std::size(kNames) == kMetrics);
		struct Stats {
			std::array<std::uint32_t, 512> bins{}; // 50 us bins; final bin is overflow.
			std::uint64_t count{}, over{}, has{};
			double maximum{}, total{};
		};
		struct Row {
			void* cls{};
			void* fn{};
			std::string name;
			std::uint64_t count{};
		};
		struct Local {
			std::array<Stats, kMetrics> stats{};
			std::array<std::uint64_t, kCounters> counters{};
			std::array<Row, 32> rows{};
			unsigned epoch{};
			std::int64_t lastFlush{};
			std::uint64_t hasTotal{}, rowsOverflow{};
		};
		thread_local Local local;
		std::atomic<unsigned> epoch{ 0 };
		std::atomic<std::int64_t> endAt{ 0 };
		std::atomic<DWORD> gameThread{ 0 };
		std::int64_t Clock() { LARGE_INTEGER t; QueryPerformanceCounter(&t); return t.QuadPart; }
		std::int64_t Frequency() { static const auto f = [] { LARGE_INTEGER t; QueryPerformanceFrequency(&t); return t.QuadPart; }(); return f; }
		bool Active(std::int64_t now) { return now < endAt.load(std::memory_order_relaxed); }
		void Prepare(std::int64_t now)
		{
			const auto current = epoch.load(std::memory_order_relaxed);
			if (local.epoch != current) {
				local = {};
				local.epoch = current;
				local.lastFlush = now;
			}
		}
		void Flush(std::int64_t now, bool force = false)
		{
			if (!local.epoch || (!force && now - local.lastFlush < Frequency())) return;
			const double elapsed = static_cast<double>(now - local.lastFlush) / Frequency();
			for (std::size_t i = 0; i < kMetrics; ++i) {
				auto& s = local.stats[i];
				if (s.maximum > 0.5) {
					const auto rank = static_cast<std::uint64_t>(std::ceil(s.count * 0.99));
					std::uint64_t sum = 0;
					double p99 = 0;
					for (std::size_t b = 0; b < s.bins.size(); ++b) {
						sum += s.bins[b];
						if (sum >= rank) { p99 = b == 511 ? s.maximum : (b + 1) * 0.05; break; }
					}
					logger::warn("TEMP-DIAG timing thread={} t={:.3f}s window={:.3f}s metric={} calls={} over0.5={} max={:.4f}ms p99upper={:.4f}ms total={:.4f}ms Has={}",
						GetCurrentThreadId(), static_cast<double>(now) / Frequency(), elapsed, kNames[i], s.count, s.over, s.maximum, p99, s.total, s.has);
				}
				s = {};
			}
			const auto& c = local.counters;
			logger::warn("TEMP-DIAG counts thread={} t={:.3f}s window={:.3f}s pad0..3={}/{}/{}/{} callbacks={} Instances={} maxObjectSlots={} GameImageFallback={} HarvestFull={} Has={} KeysVisited={} rowEvents={} rowOverflow={} messageMissing/Found/Instances={}/{}/{} RemoveGoneHas={} SlotCountsHas={}",
				GetCurrentThreadId(), static_cast<double>(now) / Frequency(), elapsed, c[6], c[7], c[8], c[9], c[10], c[0], c[1], c[2], c[3], c[4], c[5], c[11], local.rowsOverflow, c[12], c[13], c[14], c[15], c[16]);
			for (auto& row : local.rows) {
				if (row.count) logger::warn("TEMP-DIAG row thread={} class.function={} calls={}", GetCurrentThreadId(), row.name, row.count);
				row.count = 0;
			}
			local.counters = {};
			local.rowsOverflow = 0;
			local.lastFlush = now;
		}
	}

	void Pulse()
	{
		gameThread.store(GetCurrentThreadId(), std::memory_order_relaxed);
		static bool down = false;
		static bool stopped = false;
		const bool pressed = (GetAsyncKeyState(VK_F10) & 0x8000) != 0;
		const auto now = Clock();
		if (pressed && !down) {
			if (Active(now)) {
				endAt.store(0);
				Flush(now, true);
				logger::warn("TEMP-DIAG timing capture stopped by F10");
				stopped = true;
			} else {
				epoch.fetch_add(1);
				endAt.store(now + Frequency() * 180);
				Prepare(now);
				stopped = false;
				logger::warn("TEMP-DIAG timing capture started: 180 seconds; inclusive QPC timings, p99 histogram upper bounds; F10 stops; no render-frame ID");
			}
		}
		down = pressed;
		if (Active(now)) Flush(now);
		else if (!stopped && epoch.load()) {
			endAt.store(0);
			Flush(now, true);
			logger::warn("TEMP-DIAG timing capture complete; no further samples until F10");
			stopped = true;
		}
	}

	void Add(Counter counter, std::uint64_t amount)
	{
		if (!endAt.load(std::memory_order_relaxed)) return;
		const auto now = Clock();
		if (!Active(now)) return;
		Prepare(now);
		auto& value = local.counters[static_cast<std::size_t>(counter)];
		if (counter == Counter::ObjectSlots) value = std::max(value, amount);
		else value += amount;
		if (counter == Counter::Has) local.hasTotal += amount;
	}

	Scope::Scope(Metric metric) : metric_(metric)
	{
		if (metric == Metric::Count) return;
		if (!endAt.load(std::memory_order_relaxed)) return;
		const auto now = Clock();
		if (!Active(now)) return;
		Prepare(now);
		start_ = now;
		epoch_ = local.epoch;
		has_ = local.hasTotal;
	}
	Scope::~Scope()
	{
		if (!start_ || epoch_ != local.epoch) return;
		const auto now = Clock();
		const double ms = static_cast<double>(now - start_) * 1000 / Frequency();
		auto& s = local.stats[static_cast<std::size_t>(metric_)];
		++s.count;
		s.over += ms > 0.5;
		s.maximum = std::max(s.maximum, ms);
		s.total += ms;
		s.has += local.hasTotal - has_;
		++s.bins[std::min<std::size_t>(511, static_cast<std::size_t>(ms / 0.05))];
		if (metric_ == Metric::RemoveGone) local.counters[static_cast<std::size_t>(Counter::RemoveGoneHas)] += local.hasTotal - has_;
		if (metric_ == Metric::SlotCounts) local.counters[static_cast<std::size_t>(Counter::SlotCountsHas)] += local.hasTotal - has_;
		// Off-thread scopes flush their own TLS; game-thread flush is normally at Pulse.
		if (GetCurrentThreadId() != gameThread.load(std::memory_order_relaxed)) Flush(now);
	}

	void RowEvent(void* cls, void* fn)
	{
		if (!endAt.load(std::memory_order_relaxed) || !Active(Clock())) return;
		Add(Counter::RowEvents);
		for (auto& row : local.rows) {
			if (row.cls == cls && row.fn == fn) { ++row.count; return; }
			if (!row.fn) {
				row.cls = cls; row.fn = fn;
				row.name = pe::Utf8(static_cast<UE::UClass*>(cls)->GetFName().ToString()) + "." + pe::FunctionName(static_cast<UE::UFunction*>(fn));
				row.count = 1;
				return;
			}
		}
		++local.rowsOverflow;
	}
}
#endif
