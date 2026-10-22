#include "RingOverlay.h"

#include "Reflect.h"
#include "Settings.h"
#include "Ui.h"

namespace ringoverlay
{
	namespace
	{
		struct Overlay
		{
			const wchar_t*  file;
			reflect::Handle root, image, slot, texture;
			bool            textureFailed = false, shown = false;
			double          x = -1e9, y = -1e9, w = -1;
			ULONGLONG       lastBuild = 0;
		};
		Overlay g_overlay[2] = { { L"MagicWheel.png" }, { L"EquipmentWheel.png" } };   // the Magic wheel in blue, the Equipment wheel in red
		int     g_builds = 0;

		// ours, from the plugin folder: a texture the engine imports from the PNG (as the favourites star's fill)
		UE::UObject* Texture(Overlay& o)
		{
			if (auto* t = reflect::Get(o.texture)) return t;
			if (o.textureFailed) return nullptr;
			const auto path = settings::PluginFolder() / L"ImprovedWheelsPlus" / o.file;
			static auto* lib = ui::Class(L"/Script/Engine.KismetRenderingLibrary");
			ui::Call c(lib ? lib->GetDefaultObject(false) : nullptr, L"ImportFileAsTexture2D");
			void* file = c.At("Filename");
			UE::UObject* tex = nullptr;
			if (c && file && std::filesystem::exists(path)) {
				c.Set("WorldContextObject", ui::PlayerController());
				auto* s = new (file) UE::FString(path.wstring().c_str());
				tex = c.RunGuarded() ? c.Get<UE::UObject*>("ReturnValue") : nullptr;
				s->~FString();
			}
			if (!tex) {
				o.textureFailed = true;
				logger::warn("ring overlay: the Magic wheel picture could not be loaded ({}{}) - the wheel keeps the game's own picture",
					path.string(), std::filesystem::exists(path) ? "" : ", missing");
				return nullptr;
			}
			o.texture = reflect::Hold(tex);
			logger::info("ring overlay: the wheel picture is loaded from {}", path.filename().string());
			return tex;
		}

		bool Build(Overlay& o)
		{
			const ULONGLONG now = GetTickCount64();
			if (o.lastBuild && now - o.lastBuild < 2000) return false;
			o.lastBuild = now;
			auto* tex = Texture(o);
			if (!tex) return false;
			auto* root = ui::CreateWidget(L"/Script/UMG.UserWidget");
			const std::wstring n = std::to_wstring(++g_builds);
			auto* canvas = root ? ui::RootCanvas(root, (L"IwmRingCanvas" + n).c_str()) : nullptr;
			if (!canvas) return false;
			UE::UObject* slot = nullptr;
			auto*        img = ui::AddToCanvas(canvas, L"/Script/UMG.Image", (L"IwmRing" + n).c_str(), &slot);
			if (!img || !slot) return false;
			ui::Anchors(slot, 0, 0, 0, 0);
			{
				ui::Call b(img, L"SetBrushFromTexture");
				b.Set("Texture", tex);
				b.Set("bMatchSize", false);
				b.Run();
			}
			ui::Call vp(root, L"AddToViewport");
			vp.Set<std::int32_t>("ZOrder", 60);   // the Inventory menu draws its own wheel above 59: level with the wheel name (60), under the slot counts (61)
			vp.Run();
			ui::Visible(root, false);
			o.root = reflect::Hold(root);
			o.image = reflect::Hold(img);
			o.slot = reflect::Hold(slot);
			o.shown = false;
			o.x = o.y = -1e9, o.w = -1;
			logger::info("ring overlay: built (build {})", g_builds);
			return true;
		}
	}

	void Show(UE::UObject* a_gameWheelImage, Kind a_kind)
	{
		auto& o = g_overlay[a_kind == Kind::kEquipment ? 1 : 0];
		auto& other = g_overlay[a_kind == Kind::kEquipment ? 0 : 1];
		if (other.shown) {
			ui::Visible(reflect::Get(other.root), false);
			other.shown = false;
		}
		if (!reflect::Get(o.root) && !Build(o)) return;
		auto* root = reflect::Get(o.root);
		auto* slot = reflect::Get(o.slot);
		double x = 0, y = 0, w = 0, h = 0;
		if (!root || !slot || !ui::Measure(a_gameWheelImage, x, y, w, h)) {
			Hide();
			return;
		}
		if (std::abs(x - o.x) > 0.5 || std::abs(y - o.y) > 0.5 || std::abs(w - o.w) > 0.5) {
			ui::Vec2(slot, L"SetPosition", x, y);
			ui::Vec2(slot, L"SetSize", w, h);
			o.x = x, o.y = y, o.w = w;
		}
		if (!o.shown) {
			ui::Visible(root, true);
			o.shown = true;
		}
	}

	void Hide()
	{
		for (auto& o : g_overlay) {
			if (!o.shown) continue;
			ui::Visible(reflect::Get(o.root), false);
			o.shown = false;
		}
	}
}
