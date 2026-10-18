#pragma once

// ImprovedWheelMenu.ini beside the plugin. Read once at load; the compiled defaults are the shipped INI's values
// (memory: log-level-defaults-to-trace - every INI ships at info, uLogLevel=2, and the compiled default matches).

namespace settings
{
	struct Values
	{
		int  logLevel = 3;        // [Log] uLogLevel: 0 trace, 1 debug, 2 info, 3 warn (the default: warnings and errors only), 4 error
		int  entriesPerSlot = 5;  // [Wheel] uEntriesPerSlot: how many entries one slot of a wheel holds (the owner: five by default)
		bool pointFilledOnly = false;   // [Wheel] bPointFilledOnly: the stick points only at slots that hold something (as the Ammo shortcut does); off by default
		bool blockFavouriteDropSell = false;   // [Wheel] bBlockFavouriteDropSell: a favourite cannot be dropped, sold or handed over (the original mod's rule; 0: it can, and leaves the wheel)
		bool starAtLeft = true;   // [Wheel] bFavouriteStarAtLeft: the inventory's favourite star sits in the margin before the item's icon (0: the old place, a column at the row's right)
		int  starX = 6;           // [Wheel] iFavouriteStarX: pixels from the row's left edge to the star's left edge
		int  starSpace = 44;      // [Wheel] iFavouriteStarSpace: pixels the row's icon, name and columns are moved right to make room for the star
		bool armourOnWheel = true;   // [Wheel] bArmourOnWheel: armour and clothing can be put on the Equipment wheel (0: only shields, as the original mod)
		bool rememberWheel = true;   // [Wheel] bRememberWheel: the radial opens on the wheel it was left on (0: always on the Equipment wheel)
		bool chooseOnClose = false;   // [Wheel] bChooseOnClose: closing the wheel (its button let go or pressed again) uses the pointed slot, as RB / A would
		bool centreRestSnap = true;   // [Wheel] bCentreRestSnap: the right stick back at rest points at no slot
		int  restSnapMs = 150;        // [Wheel] uRestSnapMs: how long at rest before the pointer lets go
		bool ammoWheel = true;        // [AmmoWheel] bEnabled
		int  ammoButton = 0x0008;     // [AmmoWheel] uButton: XInput button mask (8 = D-pad right), with a bow held
		bool ammoCentred = false;     // [AmmoWheel] uPosition: 0 = a half wheel on the middle of the right-hand edge (the original), 1 = the whole ring in the middle of the screen, like the other wheels
		int  ammoScalePercent = 100;  // [AmmoWheel] uScalePercent
		struct Display   // [Display]: where things sit on the wheel and how big they are (percent; 0 = automatic where noted)
		{
			int uiScalePercent = 0;     // 0 = the game's own UI scale; else the scale mouse clicks are converted with (133 = 1.33)
			int cellRingPercent = 71;   // how far out the cells' centres are, as a share of the wheel picture's half-width
			int cellHitPercent = 24;    // how close to a cell's centre a click counts, the same measure
			int countTextPercent = 60;  // the "2/5" counters' text size
			int countBelowPercent = 11; // how far below its cell a counter sits, same measure as the ring
			int centreTextPercent = 70; // the Magic wheel's centre name: text size
			int centreLineChars = 26;   // the centre name's line length, in characters
		} display;
		int  equipmentWheelButton = 0;   // [Wheel] uEquipmentWheelButton: XInput mask that opens the radial straight on the Equipment wheel (0 = none)
		int  magicWheelButton = 0;       // [Wheel] uMagicWheelButton: the same for the Magic wheel
		int  ammoWheelButton = 0;        // [Wheel] uAmmoWheelButton: the same for the Ammo wheel (a bow held)
		int  animSpeedPercent = 100;  // [Wheel] uAnimSpeedPercent: the radial's pop-up / pop-out animations played at this speed (100 = the game's, 300 = three times as fast)
		int  wheelButton = 0x0100;    // [Wheel] uWheelButton: XInput mask of the button that opens the game's quick-key wheel in gameplay (256 = LB, the game's default binding of IA_Game_QuickKeys_OpenMenu, the default here; 2 = D-pad down, the original plugin's assumption)
		bool dpadDownPanel = false;    // [Controller] bDpadDownPanel: in the inventory / magic menu a hold on D-pad down presses the game's own Show Shortcuts (L3) for you, and a tap is replayed after the release; 0 = D-pad down stays the game's own and L3 opens the panel natively
		struct Keyboard   // [Keyboard]: a virtual controller - each key presses one controller control (virtual-key codes, 0 = none)
		{
			bool enabled = true;
			int  lb = 0x23, rb = 0xDC, lt = 0xBC, rt = 0xBE;                    // End, backslash, comma, period
			int  a = 0, b = 0, x = 0x2E, y = 0;                                 // Delete = X
			int  dpadUp = 0, dpadDown = 0, dpadLeft = 0xDB, dpadRight = 0xDD;   // [ and ]
			int  l3 = 0x24, r3 = 0, start = 0, select = 'F';                      // Home = L3, F = Select
			int  stickUp = 0, stickDown = 0, stickLeft = 0, stickRight = 0;     // the right stick (the mouse points otherwise)
			std::array<int, 8> slots{ 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68 };      // Numpad 1-8: slot N (gameplay: use it; Inventory / Magic menu: assign the highlighted row to it)
			std::array<int, 8> slotsAlt{ 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38 };   // the number row 1-8, the same
			int  equipmentWheel = 0, magicWheel = 0, ammoWheel = 0;   // a key that opens the radial straight on that wheel (0 = none)
			bool padWhenNone = true;        // with no controller connected, hand the game an idle connected pad so it polls every frame and the keys reach the rules
			bool replaceQuickKeys = true;   // take the game's own keyboard quick keys (IA_Game_QuickKeys_Keyboard_N) off the keys, so only the plugin's slot keys act
		} keyboard;
		int  favouriteButton = 0x0020; // [Controller] uFavouriteButton: XInput mask that favourites in the inventory / magic menu (32 = Select/Back; 0x8000 = Y, the original, with the game's sort moved to the left stick click; 0 = none)
	};

	// Loads <plugin folder>\ImprovedWheelMenu.ini; missing file or keys keep the defaults, and the log says which.
	void Load();
	const Values& Get();
	void SetAmmoButton(int a_mask);   // a rebind on the Controls page: kept in memory and written to [AmmoWheel] uButton
	std::filesystem::path PluginFolder();   // ...\OblivionRemastered\Binaries\Win64\OBSE\Plugins
}
