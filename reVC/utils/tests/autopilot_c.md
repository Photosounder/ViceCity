`AutoPilotSystem.c` contains the complete former `CAutoPilot` constructor, speed
modification, path-node removal, cruise-speed getter and compatible save/load
operations. `AutoPilot.h` defines a plain C record with all 33 original fields,
including the five bitfields and eight path pointers. Its enum names and values
remain unchanged. The existing timer-header dependency is retained.

Both native vehicle constructor paths explicitly initialize the record before
their original body statements. The later by-value members have only empty vector
constructors, as checked against the saved sources. Tests preserve the timestamp
captured after the native base constructor, including the early anti-reverse time
and later mission-time update. The C initializer follows the original assignment
sequence and adds no blanket clearing. Original constructor omissions are
indeterminate C++ state; comparisons inspect defined constructor results and seed
omitted fields before subsequent operations, rather than requiring an incidental
optimized representation of uninitialized storage.

Native compile-time discovery derives the original complete autopilot layout and
vehicle layout from an immutable source tree. The C record retains its 120-byte
x86 and 160-byte x64 extent, and the containing vehicle retains its 712-byte x86
and 880-byte x64 extent. All addressable autopilot offsets and eight containing
vehicle anchor offsets match. Every original field type is checked, and C11
compilation independently checks the record offsets. Native probes also prove
that the new record has trivial default construction, copy and destruction.
The old optional `VALIDATE_SIZE(CAutoPilot,0x70)` annotation is retained; its
112-byte expectation was already stale before conversion and is not enabled in
the normal build configurations.

The speed policy uses the existing full C curve implementation and preserves its
float operations, signed directions/lanes, minimum-speed choice, time rounding and
both FIX_BUGS/classic timestamp formulas. Path removal preserves the original
empty-list decrement and stale trailing pointer. Native destination consumers use
explicit vector copies. `AutoPilot.cpp` retains only five typed native path-link
reads and two vector boundary helpers, without autopilot branches or loops.

Compatible serialization remains exactly 120 bytes on both architectures. Every
field copy, zeroed gap, skipped gap and caller cursor update retains its original
order and width. Load leaves native path and target pointers intact. Tests cover
all 256 serialized flag bytes, nonzero skipped bytes, unaligned buffers, guard
regions, unchanged save-source records and optional save-size counters. An
independent packer uses the immutable original field sequence and actual widths.

Each whole original/C comparison covers 4,728 complete states with compatible
saves or 3,960 without them. The fixture uses the actual original record header,
actual original member bodies, actual C curve implementation and actual C save
primitives. It covers constructor results and integration timing, every valid
path count plus empty/negative counts, all cruise-speed bytes, special multiplier
payloads, defined speed-calculation boundaries, timer precision/wrap, trivial
record copies and full persistence. It avoids comparing indeterminate C++ object
padding or unowned bitfield padding as if they were defined gameplay state.

Clang and GCC run at O0/O2 across fixed/classic modes with compatible saves enabled,
disabled and validation enabled. Clang undefined-behavior checks run all six.
Native MSVC runs all six on x86/x64. Every original must match its corresponding
C executable. All changed native consumers and the C policy compile on both
architectures. All 48 generated MSBuild configurations compile the unique object
as C11 and preserve the existing 28 paired C/C++ object names. Full D3D9 Debug and
Release executables link successfully without being launched.

Run `utils/tests/test_autopilot.ps1` for quiet source, behavior and native checks.
Saved originals under `.diagnostics/autopilot-c-tests/before` are required.
Full-build helpers accept `--autopilot`. Portable diagnostics use static compiler
runtimes and run with only Windows directories on PATH.
