# C intrusive lists

`Lists.c` replaces the four list/node classes with C records and 16 explicit
operations. `CSector` is also a plain record. The only native pool glue consists
of two C getters in the existing `Pools.cpp` owner.

Both embedded entry-info lists now have explicit initialization and destruction.
Physical objects retain their original explicit flush followed by member cleanup;
dummy objects release membership before the entity base destructor. List copying
retains the original shallow pointer semantics.

The world registers its three static list groups at their original definition
positions. A bounded C descriptor stack initializes each group forwards and
destroys it backwards through `atexit`, preserving the original ordering of
buildings, moving entities, sector rows, sector columns and sector list members.
Each descriptor receives the address of the complete array object and uses byte
offsets, avoiding pointer arithmetic across separate nested list arrays.

The original algorithms retain first-match lookup, all-match removal, duplicate
and null items, stale removed-node links and cached successor traversal. Pool
allocation cursors, generation flags and node slot reuse are compared using the
real pool implementation. Pool exhaustion retains the original assertion contract.

Checks are isolated under `.diagnostics/lists-c-tests`, without using the user's
disposable `build` directory. The complete original implementations are compared
with the production C implementation across 12,002 states per profile, including
the literal world global definitions and their shutdown registrations. Clang/GCC
O0/O2, normal/MASTER, undefined-behavior checks and native MSVC x86/x64 runs agree.
Real native headers verify node/list/sector layouts and the embedded physical and
dummy layouts; every changed producer compiles on both architectures. All 48
generated configurations retain distinct names for the 28 existing C/C++ object
pairs. Both full D3D9 builds compile and link; the game is not launched.

Run `utils/tests/test_lists.ps1` to repeat the quiet behavior and source checks.
Use `events_project.py --lists`, `check_events_project.py --lists` and
`build_events_project.py --lists` (plus `--release`) to regenerate and build the
isolated full project.
