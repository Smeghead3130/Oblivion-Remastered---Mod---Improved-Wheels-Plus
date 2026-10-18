#include "Keyboard.h"

#include "Ammo.h"
#include "Menus.h"
#include "Pad.h"
#include "PEHook.h"
#include "QuickKeys.h"
#include "Wheels.h"
#include "Ui.h"
#include "Settings.h"

namespace keyboard
{
	namespace
	{
		std::array<bool, 256> g_down{};
		ULONGLONG             g_lastActive = 0;

		bool GameHasFocus()
		{
			const HWND fg = GetForegroundWindow();
			DWORD      pid = 0;
			if (!fg) return false;
			GetWindowThreadProcessId(fg, &pid);
			return pid == GetCurrentProcessId();
		}

		bool Down(int a_vk) { return a_vk > 0 && a_vk < 256 && (GetAsyncKeyState(a_vk) & 0x8000) != 0; }

		// a search box (the Inventory Search / Magic Search plugins') has the keyboard: digits are typing, not slot keys
		bool SearchBoxFocused()
		{
			auto* arr = UE::FUObjectArray::GetSingleton();
			if (!arr) return false;
			bool focused = false;
			arr->LockInternalArray();
			const std::int32_t n = arr->GetObjectArrayNum();
			for (std::int32_t i = 0; i < n && !focused; ++i) {
				auto* item = arr->IndexToObject(i);
				if (!item || !item->object) continue;
				auto* o = reinterpret_cast<UE::UObject*>(item->object);
				if (pe::Utf8(o->GetClass()->GetFName().ToString()) != "EditableTextBox") continue;
				if (pe::Utf8(o->GetFName().ToString()).find("OIS_SearchBox") == std::string::npos) continue;
				ui::Call c(o, L"HasKeyboardFocus");
				if (c && c.RunGuarded() && c.Get<bool>("ReturnValue")) focused = true;
			}
			arr->UnlockInternalArray();
			return focused;
		}

		// the mouse is over the wheel on screen (viewport units, as the widget is measured)
		// the mouse is reported in screen pixels, the widgets are measured in viewport units (pixels / the UI's DPI scale: the log
		// showed the cursor at the exact middle of a 2560 x 1440 screen with the wheel's middle at (960, 540)); the scale comes from the
		// game (GetViewportScale), failing that the engine's default rule of the window height over 1080, or the INI's own value
		bool MouseUnitsImpl(double& a_mx, double& a_my, double& a_scale)
		{
			auto* pc = ui::PlayerController();
			if (!pc) return false;
			ui::Call c(pc, L"GetMousePosition");
			if (!c || !c.RunGuarded()) return false;
			double scale = 1.0;
			{
				static auto* lib = ui::Class(L"/Script/UMG.WidgetLayoutLibrary");
				ui::Call vs(lib ? lib->GetDefaultObject(false) : nullptr, L"GetViewportScale");
				bool got = false;
				if (vs) {
					vs.Set("WorldContextObject", pc);
					if (vs.RunGuarded()) {
						const float v = vs.Get<float>("ReturnValue");
						if (v > 0.1f && v < 10.0f) {
							scale = v;
							got = true;
						}
					}
				}
				if (!got) {
					RECT rc{};
					const HWND fg = GetForegroundWindow();
					if (fg && GetClientRect(fg, &rc) && rc.bottom > 0) scale = rc.bottom / 1080.0;
				}
			}
			if (settings::Get().display.uiScalePercent > 0) scale = settings::Get().display.uiScalePercent / 100.0;
			a_scale = scale;
			a_mx = c.Get<float>("LocationX") / scale;
			a_my = c.Get<float>("LocationY") / scale;
			return true;
		}

		// The cell (0..7, -1 = none) under the mouse. The game points a slot from the mouse's ANGLE round the wheel's middle, so hovering
		// a cell does not point it (the owner, 2026-10-16: the clickable spots were inside the hole, not on the cells): the cells are
		// found from the wheel picture's own geometry - eight circles 45 degrees apart, 0.71 of the picture's half-width out (the
		// counters sit on the same spots) - and the mouse position the player controller reports.
		int CellUnderMouse(std::string* a_why = nullptr)
		{
			auto* wheel = quickkeys::VisibleWheel();
			auto* pc = ui::PlayerController();
			if (!wheel || !pc) {
				if (a_why) *a_why = wheel ? "no player controller" : "no visible wheel widget";
				return -1;
			}
			auto* material = ui::ChildNamed(wheel, L"quickKeys_material");
			if (!material) material = ui::FindInTree(wheel, "quickKeys_material");
			auto* image = material ? ui::ChildNamed(material, L"Image") : nullptr;
			if (!image && material) image = ui::FindInTree(material, "Image");
			double x = 0, y = 0, w = 0, h = 0;
			if (!image || !ui::Measure(image, x, y, w, h)) {
				if (a_why) *a_why = "the wheel picture could not be measured";
				return -1;
			}
			double mx = 0, my = 0, scale = 1.0;
			if (!MouseUnitsImpl(mx, my, scale)) {
				if (a_why) *a_why = "GetMousePosition failed";
				return -1;
			}
			const double half = w * 0.5, cx = x + half, cy = y + h * 0.5;
			int          best = -1;
			const double ring = settings::Get().display.cellRingPercent / 100.0;
			double       bestDist = half * settings::Get().display.cellHitPercent / 100.0;   // a cell is about 0.18 of the half-width across; a little more is taken
			for (int k = 0; k < 8; ++k) {
				const double ang = k * 45.0 * 3.14159265358979323846 / 180.0;
				const double px = cx + std::sin(ang) * ring * half, py = cy - std::cos(ang) * ring * half;
				const double d = std::hypot(mx - px, my - py);
				if (d < bestDist) bestDist = d, best = k;
			}
			if (a_why) *a_why = std::format("mouse ({:.0f}, {:.0f}) in viewport units (UI scale {:.2f}), wheel picture centre ({:.0f}, {:.0f}) half-width {:.0f}", mx, my, scale, cx, cy, half);
			return best;
		}

		bool Printable(int a_vk)
		{
			return (a_vk >= 'A' && a_vk <= 'Z') || (a_vk >= '0' && a_vk <= '9') || a_vk == VK_SPACE || (a_vk >= 0xBA && a_vk <= 0xC0) || (a_vk >= 0xDB && a_vk <= 0xDE);
		}

		// typing into a search box in the Inventory / Magic menu (looked at most every 150 ms, and only when asked)
		bool Typing()
		{
			static ULONGLONG s_at = 0;
			static bool      s_typing = false;
			const auto menu = menus::Active();
			if (menu != menus::Menu::kInventory && menu != menus::Menu::kMagic) return false;
			const ULONGLONG now = GetTickCount64();
			if (now - s_at >= 150) {
				s_at = now;
				s_typing = SearchBoxFocused();
			}
			return s_typing;
		}

		// slot N (1 = top, clockwise, 45 degrees apart) as a right-stick deflection
		void SlotStick(int a_slot, SHORT& a_x, SHORT& a_y)
		{
			const double angle = (a_slot - 1) * 45.0 * 3.14159265358979323846 / 180.0;
			a_x = static_cast<SHORT>(std::lround(std::sin(angle) * 32000.0));
			a_y = static_cast<SHORT>(std::lround(std::cos(angle) * 32000.0));
		}

		// A slot key's script, advanced a read at a time from what the game shows (never blind timing: a stick held toward a slot
		// before the radial is up turns the camera that way instead - the owner, 2026-10-16: "the character looks in those 8
		// directions"):
		//   use    - hold the wheel button until the radial is up, then the stick at the slot until it is pointed, then A
		//   assign - (the panel opened with L3 if it was not up) the stick at the slot until pointed, then A, (L3 again)
		struct Script
		{
			enum Kind { kNone, kUse, kAssign, kClick } kind = kNone;
			int       slot = 0;      // 0..7
			int       phase = 0;
			bool      openedPanel = false;
			ULONGLONG at = 0;
		} g_script;

		void Next(int a_phase)
		{
			g_script.phase = a_phase;
			g_script.at = GetTickCount64();
		}

		void End(const char* a_why)
		{
			logger::info("keyboard: slot key script ended ({})", a_why);
			g_script.kind = Script::kNone;
		}

		bool PanelUp(menus::Menu a_menu)
		{
			return quickkeys::PanelOpen() || (a_menu == menus::Menu::kMagic && pad::MagicPanelUp());
		}

		// lays the running script over this read; true while one is running
		bool RunScript(XINPUT_GAMEPAD& a_pad)
		{
			if (g_script.kind == Script::kNone) return false;
			const ULONGLONG elapsed = GetTickCount64() - g_script.at;
			const WORD      wheel = static_cast<WORD>(settings::Get().wheelButton ? settings::Get().wheelButton : XINPUT_GAMEPAD_DPAD_DOWN);
			SHORT           rx = 0, ry = 0;
			SlotStick(g_script.slot + 1, rx, ry);
			const auto stick = [&] { a_pad.sThumbRX = rx; a_pad.sThumbRY = ry; };
			const bool pointed = quickkeys::PointedSlot() == g_script.slot + 1;
			const auto menu = menus::Active();
			if (g_script.kind == Script::kClick) {   // a click is the A button on the slot the mouse already points (the game points by the mouse's angle, as by the stick's)
				const bool open = menu == menus::Menu::kInventory || menu == menus::Menu::kMagic ? PanelUp(menu) : quickkeys::RadialOpen();
				if (!open) {
					End("the wheel closed");
					return false;
				}
				a_pad.wButtons |= XINPUT_GAMEPAD_A;
				++g_script.phase;   // reads that carried the A: at least three, so the rules see a press even when the game reads the pad slowly
				if ((elapsed >= 50 && g_script.phase >= 3) || elapsed > 400) End("clicked");
} else if (g_script.kind == Script::kUse) {
				if (g_script.phase >= 1 && !quickkeys::RadialOpen()) {
					End("the radial closed");
					return false;
				}
				switch (g_script.phase) {
				case 0:   // the wheel button, until the radial shows
					wheels::KeepActive();
					a_pad.wButtons |= wheel;
					if (quickkeys::RadialOpen() && elapsed >= 10) Next(1);
					else if (elapsed > 900) End("the radial did not open");
					break;
				case 1:   // the stick at the slot, until it is pointed
					a_pad.wButtons |= wheel;
					stick();
					if ((pointed && elapsed >= 25) || elapsed > 400) Next(2);
					break;
				case 2:   // A: uses the slot and closes the wheel
					a_pad.wButtons |= wheel | XINPUT_GAMEPAD_A;
					stick();
					if (elapsed >= 50) Next(3);
					break;
				default:
					if (elapsed >= 40) End("used");
					break;
				}
			} else {
				switch (g_script.phase) {
				case 0:   // L3 opens the panel when it is not up
					if (PanelUp(menu)) {
						Next(1);
					} else {
						g_script.openedPanel = true;
						a_pad.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB;
						if (elapsed >= 120) Next(10);
					}
					break;
				case 10:  // until it shows
					if (PanelUp(menu) && elapsed >= 30) Next(1);
					else if (elapsed > 900) End("the panel did not open");
					break;
				case 1:
					stick();
					if ((pointed && elapsed >= 25) || elapsed > 400) Next(2);
					break;
				case 2:
					a_pad.wButtons |= XINPUT_GAMEPAD_A;
					stick();
					if (elapsed >= 50) Next(3);
					break;
				case 3:
					if (elapsed >= 40) {
						if (g_script.openedPanel) Next(4);
						else End("assigned");
					}
					break;
				default:   // L3 closes the panel again
					a_pad.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB;
					if (elapsed >= 120) End("assigned, panel closed");
					break;
				}
			}
			return g_script.kind != Script::kNone;
		}
	}

	bool MouseUnits(double& a_x, double& a_y)
	{
		double scale = 1.0;
		return MouseUnitsImpl(a_x, a_y, scale);
	}

	bool PresentWhenNoPad()
	{
		const auto& k = settings::Get().keyboard;
		return k.enabled && k.padWhenNone;
	}

	bool Apply(XINPUT_GAMEPAD& a_pad)
	{
		const auto& k = settings::Get().keyboard;
		if (!k.enabled) return false;
		if (!GameHasFocus()) {
			g_down = {};
			return false;
		}
		bool any = false;
		const auto press = [&](int a_vk, WORD a_button) {
			if (Down(a_vk) && !(Printable(a_vk) && Typing())) {   // a letter typed into a search box is not a controller press
				a_pad.wButtons |= a_button;
				any = true;
			}
		};
		{   // a key per wheel: that wheel is made the active one and the wheel button is held while the key is
			static int s_prev = -1;
			int        now = -1;
			const auto menu = menus::Active();
			if ((menu == menus::Menu::kNone && !menus::AnyOpen()) || quickkeys::RadialOpen()) {
				if (Down(k.equipmentWheel)) now = 0;
				else if (Down(k.magicWheel)) now = 1;
				else if (Down(k.ammoWheel)) now = 2;
			}
			if (now >= 0) {
				if (now != s_prev) wheels::SetActive(now == 0 ? wheels::Wheel::kEquipment : now == 1 ? wheels::Wheel::kMagic : wheels::Wheel::kAmmo);
				a_pad.wButtons |= static_cast<WORD>(settings::Get().wheelButton ? settings::Get().wheelButton : XINPUT_GAMEPAD_DPAD_DOWN);
				any = true;
			}
			s_prev = now;
		}
		if (Down(k.lb)) wheels::KeepActive();   // the keyboard's own wheel button: the wheel it left is the wheel it opens
		press(k.lb, XINPUT_GAMEPAD_LEFT_SHOULDER);
		press(k.rb, XINPUT_GAMEPAD_RIGHT_SHOULDER);
		press(k.a, XINPUT_GAMEPAD_A);
		press(k.b, XINPUT_GAMEPAD_B);
		press(k.x, XINPUT_GAMEPAD_X);
		press(k.y, XINPUT_GAMEPAD_Y);
		press(k.dpadUp, XINPUT_GAMEPAD_DPAD_UP);
		press(k.dpadDown, XINPUT_GAMEPAD_DPAD_DOWN);
		press(k.dpadLeft, XINPUT_GAMEPAD_DPAD_LEFT);
		press(k.dpadRight, XINPUT_GAMEPAD_DPAD_RIGHT);
		press(k.l3, XINPUT_GAMEPAD_LEFT_THUMB);
		press(k.r3, XINPUT_GAMEPAD_RIGHT_THUMB);
		press(k.start, XINPUT_GAMEPAD_START);
		press(k.select, XINPUT_GAMEPAD_BACK);
		if (Down(k.lt)) {
			a_pad.bLeftTrigger = 255;
			any = true;
		}
		if (Down(k.rt)) {
			a_pad.bRightTrigger = 255;
			any = true;
		}
		// the right stick: four keys, two together make a diagonal
		const int sx = (Down(k.stickRight) ? 1 : 0) - (Down(k.stickLeft) ? 1 : 0);
		const int sy = (Down(k.stickUp) ? 1 : 0) - (Down(k.stickDown) ? 1 : 0);
		if (sx || sy) {
			const double len = std::hypot(static_cast<double>(sx), static_cast<double>(sy));
			a_pad.sThumbRX = static_cast<SHORT>(std::lround(sx / len * 32000.0));
			a_pad.sThumbRY = static_cast<SHORT>(std::lround(sy / len * 32000.0));
			any = true;
		}
		// a slot key (the number row or Numpad 1-8; the game's own quick keys are taken off them): in gameplay it uses slot N at
		// once; in the Inventory / Magic menu it assigns the highlighted row to slot N through the assign panel - the controller's
		// own flow, so the Magic menu fills the Magic wheel and the Inventory the Equipment wheel (see RunScript)
		for (int i = 0; i < 8; ++i) {
			bool fired = false;
			for (const int vk : { k.slots[static_cast<std::size_t>(i)], k.slotsAlt[static_cast<std::size_t>(i)] }) {
				if (vk <= 0 || vk >= 256) continue;
				const bool now = Down(vk);
				fired = fired || (now && !g_down[static_cast<std::size_t>(vk)]);
				g_down[static_cast<std::size_t>(vk)] = now;
			}
			if (!fired || g_script.kind != Script::kNone) continue;
			const auto menu = menus::Active();
			if (menu == menus::Menu::kInventory || menu == menus::Menu::kMagic) {
				if (SearchBoxFocused()) continue;
				g_script = { Script::kAssign, i, 0, false, GetTickCount64() };
				logger::info("keyboard: slot key {} in the {} - assign to slot {} through the panel", i + 1, menus::Name(menu), i + 1);
			} else if (menu == menus::Menu::kNone && (!menus::AnyOpen() || quickkeys::RadialOpen())) {
				g_script = { Script::kUse, i, 0, false, GetTickCount64() };
				logger::info("keyboard: slot key {} - use slot {}", i + 1, i + 1);
			}
		}
		{   // the left mouse button on a cell of the open assign panel assigns the highlighted row to that cell
			static bool s_lmb = false;
			const bool now = Down(VK_LBUTTON);
			const auto menu = menus::Active();
			const bool inMenu = menu == menus::Menu::kInventory || menu == menus::Menu::kMagic;
			const bool where = inMenu ? PanelUp(menu) : quickkeys::RadialOpen();   // the assign panel, or the HUD radial
			if (now && !s_lmb && g_script.kind == Script::kNone && where && !quickkeys::GameMessageUp()) {   // a click on a message of the game is the message's
				const int pointed = quickkeys::PointedSlot();   // 1..8, the slot the mouse (or the stick) points at now
				if (pointed >= 1) {
					g_script = { Script::kClick, pointed - 1, 0, false, GetTickCount64() };
					logger::info("keyboard: click - A on slot {} of the {}", pointed, inMenu ? "assign panel" : "radial");
				}
			}
			s_lmb = now;
		}
		if (ammo::Open()) {   // the ammo wheel: the mouse points (hover) and chooses (click); the arrow keys step, Enter equips, Backspace closes
			static bool      s_lmb = false;
			static ULONGLONG s_aUntil = 0;
			const bool       lmb = Down(VK_LBUTTON);
			double           mx = 0, my = 0;
			if (MouseUnits(mx, my)) {
				const int entry = ammo::EntryUnderMouse(mx, my);
				if (entry >= 0) {
					ammo::PointAt(entry);
					if (lmb && !s_lmb) s_aUntil = GetTickCount64() + 70;
				}
			}
			s_lmb = lmb;
			if (GetTickCount64() < s_aUntil) {
				a_pad.wButtons |= XINPUT_GAMEPAD_A;
				any = true;
			}
			press(VK_UP, XINPUT_GAMEPAD_DPAD_UP);
			press(VK_DOWN, XINPUT_GAMEPAD_DPAD_DOWN);
			press(VK_RETURN, XINPUT_GAMEPAD_A);
			press(VK_BACK, XINPUT_GAMEPAD_B);
		}
		if (RunScript(a_pad)) any = true;
		const ULONGLONG t = GetTickCount64();
		if (any) g_lastActive = t;
		return any || t - g_lastActive < 400;
	}

	bool ScriptRunning()
	{
		return g_script.kind != Script::kNone;
	}
}
