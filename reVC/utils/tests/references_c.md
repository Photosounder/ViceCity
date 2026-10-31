Reference management now owns its entire 1,800-record pool and all seven
algorithms in References.c, compiled as C11. References.h declares a plain
two-pointer CReference record, the live array/free-list globals and C functions.
The static CReferences class is gone, including its direct replay snapshot users.

References.cpp retains thirteen typed field/query/conversion operations and four
one-call CEntity entry wrappers until those entity owners are converted. No list
algorithm remains there. The C implementation treats CEntity and CPlayerPed as
opaque types and obtains the actual list field through the typed bridge. It does
not duplicate entity layouts or assume that derived pointers share their base
address. Player queries remain repeated, and the FIX_BUGS path restores the live
focused-player slot before registering that same field address. World pruning
retains the production CPool API and all original pool queries and slot order.

references_fixture.py audits the complete immutable pre-conversion source bodies
and all ten changed source/project files, including both replay memory copies.
The public C record retains the original pointer size, alignment and offsets.
The Visual Studio project assigns distinct References.c.obj and References.cpp.obj
outputs, extending the object-collision regression to ten C/C++ source pairs.

The differential fixture links the whole saved original C++ implementation and
the complete production C implementation into separate quiet console programs.
Only the C++ owner boundary is replaced for these comparisons. Mock owners use
multiple inheritance and nonzero derived-to-base offsets; their real C++ casts
exercise the bridge without inventing a C owner layout. The fixture uses the
actual immutable config.h and Pool.h and observes every record's next/payload
addresses, every entity head, all caller slots, player fields, and original type,
player and pool query order. It covers exhaustion, duplicate registration,
building rejection, 36,000 retargeting/list operations, all three-record stale/live
masks and cleanup positions, sparse/empty world pools, and absent, stable or
changing player queries with and without an exhausted pool. A separate C11 caller
enters registration through the public opaque-entity interface.

Eight Clang/GCC O0/O2 comparisons and two undefined-behavior trap comparisons
match in both FIX_BUGS states. Five optimized MSVC/LTCG comparisons match across
Win32/Win64, including the actual VANILLA_DEFINES configuration on Win32.
Each run observes 1,235 complete states; default produces cab2a6a8183fbeec with
41,157 events, and the legacy branch produces 3b8fe2328fd12332 with 41,133 events.
These are algorithm/ABI comparisons with mock owners, not gameplay tests.

Fourteen additional compilations use the actual production C++ headers and six
owner/consumer source units plus References.c, in default and vanilla Win32.
Actual Release and Debug x64 D3D9/OpenAL project builds compile and link against
the actual owners and existing dependency libraries, with their outputs inside
build/references-c-tests. The unrelated Git hash build events are disabled.
No game window is launched. Commands, logs and immutable input snapshots are in
build/references-c-tests; build/test_references.py, test_references_native.py,
check_references_win32.py and build_references.py reproduce these checks.
