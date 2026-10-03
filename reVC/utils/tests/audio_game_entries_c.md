# Crime entry and player vehicle lookup in C

AudioGameEntries.c implements the former owner ReportCrime and FindVehicleOfPlayer
operations plus the public DMAudio crime facade as actual C11 functions. All audio
and MusicManager vehicle lookup callers use the owner-independent C function.
The suspect report calls it directly, and its old lookup wrapper is removed.
PolRadio.cpp now contains game access adapters rather than police business logic.

Crime entry retains initialization/music/wanted short circuits, the original OR
crime-type expression, cooldown check, and existing crime record kernel. The
public vector pointer ABI remains unchanged. C treats the game vector as opaque;
a C++ adapter reads its coordinates into a plain C record only after eligibility.
No C++ vector layout or reinterpretation assumption is introduced.

Vehicle lookup retains the player-vehicle call followed by the separate player
call even when a vehicle exists. Attachment is read only for no vehicle/non-null
player, followed by the original IsVehicle test. The base-to-derived conversion
remains in C++ so the C kernel does not assume the game object's base offset.

Run utils/tests/test_audio_game_entries.ps1. The generator audits both complete
bodies, public facade, five adapters and seven complete caller/header edits against
the saved sources. Original members and actual C share the existing C crime
record operations and controlled game callbacks. The fixture uses a vehicle with
a nonzero entity-base offset to check object identity and the cast boundary. A
wanted-level callback mutates coordinates to verify the original late read point.
Both live owners, lookup combinations, signed wanted values, all safe crime
indices, frame/cooldown boundaries and repeated reports are compared. An actual
C caller reaches both operations and the public facade.

Sixteen Clang/GCC O0/O2 comparisons cover default, vanilla, no-external and PS2
records. Native MSVC x86/x64 comparisons pass. Eighteen previous source audits
and 400 unique consumer compilation checks pass; native game callers compile in
both architectures. Vehicle report runtime comparisons are rerun with a controlled lookup
boundary; this fixture separately tests the actual new C lookup implementation.

Legacy invalid crime indices reaching the array access and null player/wanted
inputs reaching dereference remain unsafe and are excluded. No range check or
new gate is added. Game/backend functions are controlled callbacks; no full linked
game build or audio playback is verified.
