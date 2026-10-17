#pragma once

#include <Xinput.h>   // types only

// ============================================================================================================
// The keyboard as a virtual controller (the owner, 2026-10-16: "Its better to do this all as virtual controller and map
// there. We are going to have a lot of issues if we don't do that"): the keys in the INI's [Keyboard] section press the
// controller control they are named for, laid over the real pad on the game thread, so every rule in Pad.cpp (the wheels,
// the assign panel, favouriting, clearing a slot, the ammo wheel) runs for them exactly as for a controller. With no pad
// connected the read is handed over as a connected pad while a bound key is down (and for a moment after).
// A slot key (Numpad 1-8) plays "wheel button + the right stick at slot N, then A", so the slot is pointed and used at once.
// ============================================================================================================

namespace keyboard
{
	// lays the keys that are down over a_pad; true when any bound key is active (so a pad-less read is worth handing over)
	bool Apply(XINPUT_GAMEPAD& a_pad);
	// true when, with no pad connected, the game should be handed an idle connected pad all the time (the game polls a missing pad only
	// now and then, so the keys would otherwise reach the rules a few times a minute)
	bool PresentWhenNoPad();
	// the mouse position in the UI's viewport units (the unit the widgets are measured in); false when it cannot be read
	bool MouseUnits(double& a_x, double& a_y);
	// a slot key's script (wheel button, the stick at slot N, A) is running: its stick is meant exactly, never moved to another slot
	bool ScriptRunning();
}
