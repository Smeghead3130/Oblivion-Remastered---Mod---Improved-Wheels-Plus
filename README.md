# Improved Wheel - Keyboard and Better Controller Support (Oblivion Remastered)

A fork of [Improved Wheel Menu](https://github.com/ApocryphaRealm/ImprovedWheelMenuOR) by ApocryphaRealm, version **1.1.3** of this fork.

**The main addition is keyboard support:** the whole wheel menu (the inventory, Magic and ammo wheels, favouriting, the assign panel) works from the keyboard, with or without a controller. Every key is configurable in the `[Keyboard]` section of `ImprovedWheelMenu.ini`, and so is every other setting. On top of that it adds the Select/L3/LB controller rules, clearing a slot, spells you made, slot counters and names on the Magic wheel, and per-character wheels. **Clear your existing wheel before installing** (instructions in [`dist/README.txt`](dist/README.txt)). See [`CHANGELOG.md`](CHANGELOG.md). The DLL keeps the original's name, so install it instead of the original. Original authorship and commit history are kept; this fork is GPL-3.0-or-later like the original.

---

# Improved Wheel Menu (Oblivion Remastered)

An OBSE64 plugin that makes Oblivion Remastered's own quick-key radial a full wheel menu: eight slots that each hold
several items (LT / RT choose, RB or A use), a Magic wheel of its own for spells, and an ammo wheel for arrows while a
bow is held. The game's radial, prompts and quick keys are used throughout. The package README is
[`dist/README.txt`](dist/README.txt); the history is [`CHANGELOG.md`](CHANGELOG.md).

## Build

Requires [XMake](https://xmake.io) 3.0+ and a C++23 compiler (MSVC or clang-cl).

```bat
git clone --recurse-submodules https://github.com/ApocryphaRealm/ImprovedWheelMenuOR
cd ImprovedWheelMenuOR
xmake build
```

The DLL is written to `build/windows/x64/releasedbg/ImprovedWheelMenu.dll`; the shipped files are under `dist/`.

## Licence

GPL-3.0-or-later ([`dist/LICENSE`](dist/LICENSE), [`dist/NOTICE.md`](dist/NOTICE.md)). Built on
[CommonLibOB64](https://github.com/libxse/commonlibob64) and its plugin template (GPL-3.0, with the modding exception in
`EXCEPTIONS`); other linked components are listed in [`dist/THIRD_PARTY_NOTICES.md`](dist/THIRD_PARTY_NOTICES.md).
