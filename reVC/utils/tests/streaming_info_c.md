# C streaming-info records

`StreamingInfo.h` exposes the seven original fields, stream flags and load states
without the C++ manager dependency. `StreamingInfo.c` owns all six original
operations. No C++ methods or native bridges remain for `CStreamingInfo`.

The larger `CStreaming` manager keeps its existing canonical array and four list
sentinels. Its initialization, allocation and streaming policy are unchanged;
every record method call now invokes the C API. The record remains trivial to
initialize, copy and destroy, with its original 20-byte x86 and 32-byte x64 layout.

Disc-position lookup leaves outputs untouched for zero-size entries. Successful
lookup still writes position first and then reads and writes size. This ordering
also preserves cases where outputs share storage or point into the record. List
insertion follows the specified link, and removal clears both record links after
repairing its neighbors. Flags, load state, next ID and disc metadata survive list
transfers.

Quiet differential checks use the literal original methods and independent C11
callers. They cover 8,578 observed states, every flag byte and load-state byte,
32-bit position/size boundaries, all output aliases, and transfers between two
sentinel lists. Clang/GCC O0/O2, undefined-behavior checks and native MSVC x86/x64
agree. Real native headers verify every field type/offset, enum values, trivial
lifetime and the canonical manager array/sentinel types. The complete manager
compiles in normal and MASTER modes on both architectures.

Checks live under `.diagnostics/streaming-info-c-tests`. All 48 generated build
configurations compile the new source as C11 and retain unique object names for
the 28 existing C/C++ source pairs. Both complete D3D9 configurations compile and
link without launching the game.

Run `utils/tests/test_streaming_info.ps1` for quiet behavior and native source
checks. Regenerate/build the isolated full project with `events_project.py
--streaming-info`, `check_events_project.py --streaming-info` and
`build_events_project.py --streaming-info` (plus `--release`).
