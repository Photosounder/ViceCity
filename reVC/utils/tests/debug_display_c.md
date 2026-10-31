`DebugDisplaySystem.c` owns all seven former `CDebug` operations, all six canonical
queue objects and the existing `gbDebugStuffInRelease` flag. `Debug.h` is a C
header with plain screen-string, line and vector records. Screen strings retain
their 264-byte extent and line records retain their 32-byte extent, both aligned
to four bytes with the original field offsets. Every caller uses the C API.

The C implementation retains the complete 15-row text ring, the 80-byte row limit,
both 100-entry queues and all original display setup. Initialization resets only
the text cursor. Text writes preserve stale tail bytes. Screen-string writes keep
the original `strncpy` zero padding and nontermination for inputs of 256 bytes or
more. Full queues reject new entries before reading their contents. The tests
observe truncated records without sending unterminated strings to a renderer.

Every original font operation, print position, color and Unicode conversion point
is retained, including fixed/classic setup differences and complete MASTER
suppression of text-buffer display. Screen-string display still runs in MASTER.
All loops retain live queue/count reads. Screen queues reset after the font flush;
line queues reset after rendering. The two persistent render-state changes occur
even for an empty line queue. Line rendering uses the existing C line API directly.

`Debug.cpp` retains 21 typed native font, scratch-buffer, screen-printer and
render-state primitives. It contains no queue branch or loop. Vector arguments are
copied through a native boundary helper at existing C++ callers. Screen-printer
coordinates retain their native signed-short narrowing. The native APIs and
provider implementations remain unchanged.

Each whole original/C comparison covers 3,284 full state snapshots, all initialized
queue and scratch bytes, all font/render state and ordered native effects. Cases
include static initialization, partial reset, ring wrap, exact/truncated lengths,
valid text aliases, queue saturation, null input rejected by a full queue, special
float endpoint payloads, empty/full flushes and reentrant append/clear callbacks.
The actual original ASCII-to-Unicode conversion body is used. Private original
state is exposed only in the test header to permit equivalent observations.

Clang and GCC run at O0/O2 for fixed, classic, MASTER fixed and MASTER classic
profiles. Clang undefined-behavior checks run all four. Native MSVC runs all four
on x86/x64, and each original must match its corresponding C executable exactly.
Native probes verify every canonical storage type, constant and record layout.
The complete owner, C implementation and every changed consumer compile on both
architectures. All 48 generated MSBuild configurations compile the unique object
as C11 and retain the existing 28 paired C/C++ object names. Full D3D9 Debug and
Release executables link successfully without being launched.

Run `utils/tests/test_debug_display.ps1` for quiet source, behavior and native
checks. Saved originals under `.diagnostics/debug-display-c-tests/before` are
required. Full-build helpers accept `--debug-display`. Portable diagnostics link
static compiler runtimes and run with only Windows directories on PATH.
