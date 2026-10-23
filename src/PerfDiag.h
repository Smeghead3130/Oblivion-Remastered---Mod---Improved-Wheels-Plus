#pragma once
// TEMP-DIAG: compile only with xmake f --iwp_timing=y. No release instrumentation.
#include <cstdint>

namespace perfdiag
{
	enum class Metric {
		MagicTick, MagicTickMagic, MagicTickAmmo, MagicTickInventory, MagicTickMagicPanel,
		GameImage, VisibleWheel, ShownOnScreen, Measure, EnsureLoaded, MagicSlots, HighlightedItem,
		GameMessageClassMissing, GameMessageClassFound, Chained, Previous0, Previous1, Previous2, Previous3,
		KeyboardApply, Rewrite, FrameCallback, ControlsTick, Unbind, Follow, Harvest, FindSaveData,
		Instances, Save, RemoveGone, SlotCounts, Keys, OffsetOpacity, KeyPoll, AssetLoad,
		BoostAnimations, RingShow, FindInTree, SessionCurrent, TesPass, Count
	};
	enum class Counter { Instances, ObjectSlots, GameImageFallback, HarvestFull, Has, KeyItems,
		Pad0, Pad1, Pad2, Pad3, FrameCallbacks, RowEvents, MessageMissing, MessageFound,
		MessageInstances, RemoveGoneHas, SlotCountsHas, Count };
#ifdef IWP_TEMP_TIMINGS
	void Pulse();
	void Add(Counter counter, std::uint64_t amount = 1);
	void RowEvent(void* cls, void* fn);
	class Scope {
	public:
		explicit Scope(Metric metric);
		~Scope();
		void Label(Metric metric) { metric_ = metric; }
	private:
		Metric metric_;
		std::int64_t start_{};
		std::uint64_t has_{};
		unsigned epoch_{};
	};
#else
	inline void Pulse() {}
	inline void Add(Counter, std::uint64_t = 1) {}
	inline void RowEvent(void*, void*) {}
	class Scope {
	public:
		explicit Scope(Metric) {}
		void Label(Metric) {}
	};
#endif
}
