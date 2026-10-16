Improved Wheel Menu - Controller and Keyboard Support
=====================================================
Version 1.1.0

A modified build of Improved Wheel Menu (Oblivion Remastered) by ApocryphaRealm (GPL-3.0). It replaces the original
ImprovedWheelMenu.dll and keeps its file and setting names, so install it instead of the original, not beside it.
What this build adds to the original 1.0.1 is listed under ADDED IN THIS BUILD below and in CHANGELOG.md.

Oblivion Remastered's own quick-key radial, made into a full wheel menu: eight slots that each hold several items, a
Magic wheel of its own for spells, and an ammo wheel for your arrows while a bow is held. The game's own radial, button
prompts and quick keys are used throughout; nothing of the vanilla interface is replaced.

ADDED IN THIS BUILD
-------------------
  * KEYBOARD SUPPORT, the main addition: the whole wheel menu works from the keyboard, with or without a controller. Every key is
    configurable in the [Keyboard] section of ImprovedWheelMenu.ini (and so is every other setting): each key presses the controller
    control it is named for, so every rule below works from the keyboard. Defaults: End = LB (opens the wheel), [ and ] = D-pad left / right,
    comma / period = LT / RT, backslash = RB, F = Select (favourite), Home = L3 (assign panel), Delete = X; number keys 1-8 and Numpad 1-8
    are slot keys (in gameplay they point at that slot and use it; in the Inventory / Magic menu they assign the highlighted item or spell to it).
  * Hold X (0.6 s) on a pointed slot of the assign panel to clear the whole slot; L3 opens the assign panel.
  * Select (uFavouriteButton) favourites the highlighted item or spell; LB (uWheelButton) opens the wheel; the game's Y sorting is kept.
  * Spells you made at the Altar of Spellmaking can go on the Magic wheel, also when two share a name.
  * "2/5" slot counters on the Magic wheel too, and the wheel's centre names the pointed spell or item (also names you typed).
  * The whole wheel from the keyboard: every key of the [Keyboard] section is rebindable; number keys 1-8 and Numpad 1-8 are slot keys.
  * Buttons that open a wheel directly (uEquipmentWheelButton, uMagicWheelButton, uAmmoWheelButton), the radial's animation speed
    (uAnimSpeedPercent), and where the ammo wheel opens (uPosition).
  * Wheels are kept per character, and anything a loaded save cannot have (a spell or item that character does not own) is taken off
    the wheel so nothing tries to use a form that is not there.
  * Works with the Oblivion Menu Search and Oblivion Search Keyboard mods: while the on-screen keyboard is open the wheel's menu rules
    stand aside.

WHAT YOU GET
------------
  * The inventory wheel: hold the game's quick-key button to open the radial and point with the right stick. Each of
    the eight slots holds up to five items (uEntriesPerSlot); the counter under the wheel shows which place the shown
    item has and how many the slot holds. LT and RT choose the item within the pointed slot, RB or A use it, and
    D-pad left and right switch between the inventory and Magic wheels. Pointing alone never uses anything, letting
    go of the wheel button uses nothing, and B closes the wheel.
  * The centre rest snap (bCentreRestSnap, on by default): with it on, the stick back at rest in the middle points
    no slot, so letting go of the stick never picks one by mistake. With it off a slot stays pointed until you pick
    (A or RB) or close the wheel.
  * Putting things on the wheel: the favourite button (Select by default, uFavouriteButton) on an item in the
    inventory, or on a spell in the magic menu, makes it a favourite and puts it on its wheel; again takes it off. A
    favourite cannot be sold or dropped. To place it yourself, press L3 for the game's assign panel, point a slot,
    choose its place with D-pad left and right, and press A. Hold X (0.6 s) on a pointed slot of the panel to clear the
    whole slot. The game's own sorting stays on Y.
  * Spells you made at the Altar of Spellmaking go on the Magic wheel like any other, also when two share a name (the
    menu row's index identifies the spell).
  * Each slot shows "2/5" (the place of the shown entry out of the slot's cap) on both wheels, and the centre of the wheel
    names the pointed spell or item.
  * Keyboard (INI [Keyboard], every key rebindable): the keyboard is a virtual controller - each key presses the
    controller control it is named for, so every rule here works from the keyboard. Defaults: End = LB (wheel button),
    [ and ] = D-pad left / right (switch wheels; ammo wheel with a bow), comma / period = LT / RT (step entries),
    backslash = RB (use), F = Select (favourite; not while a search box has the keyboard), Home = L3 (assign panel), Delete = X (hold to clear a slot).
    The number row and Numpad 1-8 are slot keys: in gameplay they point at slot N and use it at once; in the Inventory /
    Magic menu they assign the highlighted item or spell to slot N. The game's own keyboard quick keys are taken off
    those keys (bReplaceQuickKeys=0 leaves them); the radial still opens with the game's own key (hold R).
  * The Magic wheel: a separate wheel for spells, always drawn as itself. A pick makes the spell the one the cast
    button casts, and the HUD's quick-magic picture follows it.
  * The ammo wheel: with a bow held, D-pad right opens a half wheel on the right-hand edge with the arrows you have
    put on it; the right stick or D-pad up and down point, the button again or A equips, B closes. Its button is
    rebindable from its own row on the game's Controls page.
  * Settings in ImprovedWheelMenu.ini: entries per slot, the rest snap and its time, the ammo wheel on/off, button
    and size, the log level.

KNOWN ISSUES
------------
  * There is no settings page; the INI holds the few settings there are.
  * Controller first: the wheel is the game's controller radial. On keyboard and mouse the game's own quick keys work
    as before.

BEFORE YOU INSTALL: CLEAR YOUR EXISTING WHEEL
---------------------------------------------
If you already use the original Improved Wheel Menu (or an earlier build), empty your wheels first. This build keeps the wheels per
character, and a wheel saved by the old version is taken over by the first character that loads, so start clean to avoid old or
unwanted entries landing on it. With the old version still installed:
  1. Open the Inventory and press Y on every favourited item (the star) to take it off its wheel: weapons, armour, potions, arrows.
     Y again toggles it, so a plain item has no star when it is off the wheel.
  2. Open the Magic menu and press Y on every starred spell the same way.
  3. Hold the wheel button and open each wheel (Equipment, Magic, and Ammo with a bow in hand) to check that every slot is empty.
Do this for each character you play, then close the game and install this build. If you have already installed it, the same can be
done here: press the favourite button (Select) on a starred item, or press L3 in the Inventory / Magic menu, point a slot and hold X
for 0.6 s to clear the whole slot.

INSTALLATION
------------
  * Everything goes under OblivionRemastered\Binaries\Win64\OBSE\Plugins\: ImprovedWheelMenu.dll, .ini and the
    ImprovedWheelMenu folder (the favourite star's fill picture).
  * Install with your mod manager or drop the OblivionRemastered folder over the game's own. Mod Organizer 2 users
    need the folder installed at the game's root (Root Builder or an equivalent).

DEBUGGING
---------
  Send the log with any bug report: Documents\My Games\Oblivion Remastered\OBSE\Logs\ImprovedWheelMenu.log. Set
  [Log] uLogLevel=1 in ImprovedWheelMenu.ini for more detail first.

REQUIREMENTS
------------
  * OBSE64 (Nexus 282), 0.2.2 or newer
  * Address Library for OBSE Plugins (Nexus 4475)

LICENCE
-------
  GPL-3.0-or-later (LICENSE, NOTICE.md). Copyright (C) 2026 ApocryphaRealm; the additions of this build Copyright (C) 2026 cfa911.
  Built on CommonLibOB64 (GPL-3.0); spdlog and the other linked components are listed with their notices in THIRD_PARTY_NOTICES.md.
  Source of this build: https://github.com/cfa911/ImprovedWheelMenuOR-Controller-Keyboard-Support
  Original: https://github.com/ApocryphaRealm/ImprovedWheelMenuOR
