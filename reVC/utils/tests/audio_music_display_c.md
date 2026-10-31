Radio-station display
=====================

`MusicDisplay.c` owns the complete former `DisplayRadioStationName` operation.
Its C++ member and declaration are removed, and the game renderer enters C
directly. Stored record fields remain unchanged.

The text pointer and byte counter remain shared across music owners. Station
selection, signed/byte retune arithmetic, both MP3 checks, radio-off missing-text
handling, pause/widescreen/replay short circuits and font-call order retain their
original behavior. Subtracting frames still wraps the byte counter. Different
pointers containing identical text still reset the timer.

Eighteen typed boundaries expose remaining C++ text, camera/timer, screen
scaling and font interfaces. Screen readers retain the original macros. Text
uses the original 16-bit code units and pointer identity. The missing-text
buffer declaration retains C++ linkage, matching its existing definition.

Validation
----------

Twenty-eight comparisons cover Clang/GCC, O0/O2 and seven shapes: default,
vanilla, no radio-off text, no previous-station scrolling, PS2, no aspect
correction and forced PC scaling. Eight native MSVC comparisons cover x86/x64
and four shapes with whole-program optimization. Three trapping undefined-
behavior/float-cast-overflow comparisons cover default, vanilla and no aspect
correction.

Each comparison exercises 55296 repeated calls across two owners and vehicles.
Callbacks mutate live inputs after capturing arguments/returns; another pass
repeats calls without resetting state. Observers compare complete music and
sample-provider records, vehicle stations, screen inputs, text pointer identities,
retune state and ordered font arguments. The baseline uses the exact original
screen macros and ScaleY body. Pure readers introduce no artificial callbacks.
Independent assertions check expiry across owners, text-pointer resets, byte
wrap, sixty-one rendered calls before expiry and two prints per font draw.

The broad matrix checks 125 actual consumers across ten configurations. Ten
actual C++ consumers, including the game-render caller, compile with MSVC on
both architectures. Ten thread/decoder guards and 80 earlier source audits pass.
An additional Clang game-caller check fails on external rouziclib function-pointer
conversions; the preserved original caller and header fail on the same errors.
Full game linking and interactive rendering remain outside these quiet checks.
