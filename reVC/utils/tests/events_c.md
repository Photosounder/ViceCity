The complete event list and accident manager now execute in `src/core/GameplayEvents.c`.
`EventList.h` and `Accident.h` declare plain records and 17 public C operations. The
former event overloads have distinct names, manager operations take an explicit
manager pointer, and record construction uses explicit initialization functions.
Canonical queue and manager storage is defined in C. Typed C++ leaves retain only
native entity, pool, collision and wanted-system accesses while those owners remain
C++.

The event record retains its original x86/x64 sizes of 36/48 bytes. Accident records
remain 12/16 bytes and the twenty-record manager remains 240/320 bytes. Native
probes check every field offset, alignment and the original constructor lifecycle.

Whole original/C comparisons cover every event and crime mapping, pool downcasts,
queue capacity and duplicate refresh, strict time and distance boundaries, timeout
wrap, two-step clearing, stale criminal references, reference callbacks, reward
callbacks, fresh player lookups, injury eligibility and medic selection. They run
with and without `FIX_BUGS` and `SQUEEZE_PERFORMANCE`, using Clang, GCC and native
x86/x64 MSVC. Independent C callers exercise every public operation. Reference
models deliberately use nonzero entity and player base offsets.

Checks and immutable original sources live under `.diagnostics/events-c-tests`,
outside the disposable `build` directory. Diagnostic executables use static compiler
runtimes; portable comparisons run with only Windows directories on PATH. Windows
error dialogs are disabled for quiet console checks. No game or other GUI is launched.

Run `utils/tests/test_events.ps1` from PowerShell to repeat the source, behavior,
sanitizer, ABI and actual-consumer checks. The saved originals must be retained for
the historical comparison. `events_project.py` can generate an isolated full MSVC
project under the diagnostics directory using the local SDK and dependency paths.
