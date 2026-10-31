The on-screen timer/counter controller now uses three plain C records, ten C11
operations with explicit record pointers, and one canonical C-owned
OnscreenTimer_state. The CUserDisplay static timer member and all ten C++ methods
are removed. All nine consumers use the C instance/API. Unrelated user-display,
HUD rendering and script interpreter behavior is retained.

OnscreenDisplay.c contains each complete original algorithm. OnscreenTimer.cpp
contains four typed native readers: script variable pointer, replay predicate,
HUD visibility and camera widescreen flag. The C controller calls the existing
Timer and DMAudio C APIs directly. The script variable pointer remains live and
is captured once at the original lookup point. CMake/Premake source globs include
the new C file; Visual Studio registers it explicitly as C11 under a unique
basename, preserving all existing distinct C/C++ object outputs.

Before editing, native x86/x64 probes established actual layouts of 56 bytes for
the clock, 60 for the counter and 240 for the controller, each aligned to four
bytes. The old disabled clock/controller annotations said 60/244. New probes
compare every original field offset/type, both complete array extents, all ten
original member signatures and corresponding explicit C receiver signatures.
All three records retain their trivial construction, copying and destruction.
The canonical object retains static zero initialization. No constructors,
polymorphism or object-layout casts are introduced.

Twenty portable original/C comparisons cover Clang/GCC O0/O2 with default, plain,
MASTER, unfixed and GL profiles. Nine MSVC O2/LTCG comparisons cover default,
MASTER, unfixed and GL on both architectures plus Win32 vanilla. Five Clang O1
comparisons trap undefined behavior and float-cast overflow. Every pair produces:

    states=12575 checks=1954 variables=5682 replays=1198 sounds=22 callbacks=6 hash=f80c4a8c3ae01d29

The oracle compiles all ten unmodified original bodies and the original record
declarations. After conversion, it compiles the actual production C file and all
four complete native readers. Each path enters through a separate C11 caller;
the original path uses opaque pointers to original classes without inspecting
their representation from C. Native comparisons use real project common/types,
config, Timer/DMAudio declarations, complete original script variable lookup,
variable-space-size and byte-reading primitives. Fixture owners observe script,
replay and audio boundaries. Native compilation of the real adapter and every
updated C++ caller independently verifies their actual game owner bindings.

Checks include static zero state, repeated partial initialization, all ten label
bytes, stale display-buffer tails and padding, null/empty/exactly ten-byte/long
labels, counter occupancy by label versus clock occupancy by offset, duplicate
clear matches, clearing offset zero, and independent local controller storage.
Countdown tests distinguish zero from negative expiry, truncating millisecond
steps, crossing the twelve-second boundary, multiple seconds per update,
widescreen suppression and incrementing clocks. Display tests include signed
counter values, unsigned clock wrapping, minute modulo, direct entry methods,
HUD-hidden retained flags and ignoring replay/disabled flags during formatting.
Boundary callbacks mutate direction, timestep, offset, later entries, disabled
state and audio-visible state at the original call sites. A deterministic
12,000-operation sequence compares complete interleaved controller behavior.
Inputs retain the original valid-index, aligned-variable and representable
arithmetic preconditions. The original format strings and fixed-size strncpy
semantics are preserved, including their existing signed-looking counter output
and lack of forced termination for full labels.

Fifty production source compilations cover every updated C++ unit and the C
controller across x86/x64 D3D9/GL and Win32 vanilla. Actual x64 D3D9/OpenAL Debug
and Release projects compile and link. Historical source audits retain complete
relocated producer proofs, including the earlier Timer consumer. Immutable
originals, full compiler/build logs and result records live under
build/onscreen-timer-c-tests. No graphical test or game executable was launched.

Run utils/tests/test_onscreen_timer.ps1 with optional -Sanitized, -Native,
-Consumers, -BuildProject and -AuditPrevious switches.
