# C cached collision polygons

`StoredCollPoly.c` owns the complete C record, vertex setters and getters, and the
entire cached vertical-line validation policy. The native `CCollision` method is
removed. Its pedestrian caller and two vehicle callers use the C interface.
The scalar record remains 40 bytes aligned to 4, with 12-byte vertices and valid
at 36. The VU record remains 64 bytes aligned to 16, with 16-byte vertices and
valid at 48. Native COFF constants compare pedestrian/vehicle sizes, polygon
offsets and following-member offsets before and after on x86 and x64.

The original default constructors perform no writes; static records retain their
zero-filled storage. Ordinary record copying remains trivial. SetVertex captures
the complete vertex representation before writing, including an initialized VU
fourth lane. GetVertex returns exact XYZ bits. The native conversion adapter keeps
the original CVector-to-CVuVector conversion, whose fourth lane is unspecified.
Tests deliberately exclude that unspecified lane after XYZ conversion and exclude
aggregate tail padding after ordinary assignment.

All 21 native vertex writes evaluate their complete native expression before the
C update. The two temporary-record assignments remain ordinary record copies.
Native matrix expressions use actual native vector values built from C XYZ
coordinates. Position references retain native temporaries through their complete
surrounding call and support aliases into actual cached vertices.

The C scalar policy keeps the original plane setup, same-side test, line
intersection grouping, all six projection directions, edge tests and cache flag
updates. The VU policy keeps the original cross-product direction, normalization
fallback and uncompressed plane distance. A single native leaf transfers aligned
quadwords into actual VU objects, executes the existing routine and captures its
original outputs even on failure. Fixed VU builds invalidate a failed cache;
classic VU builds retain its flag. Success changes only contact point XYZ.
The existing VU emulator, assembly and result-fetch helpers are unchanged.

Independent strict C11 callers compare the complete original method against C
over 102,877 states. Coverage includes 65,536 arbitrary vertex representations,
self-aliasing, native conversions, default/static lifetimes, ordinary copies,
all dominant projection directions, both windings, edges, degenerate triangles,
reversed and zero-length lines, repeated cache transitions, cached-vertex aliases,
native position temporaries, 16,384 random geometries and nonfinite positions.
Every unrelated contact field and cache byte is checked. Only arithmetic NaN
payloads are canonicalized in result summaries.

Clang/GCC O0/O2, UBSan and MSVC x86/x64 comparisons run in float, compressed and
VU profiles, each with fixed and classic behavior. Both staged and production
sources pass 44 native source/layout checks. All 48 evaluated configurations
select C11 and retain distinct objects for the 28 C/C++ basename pairs.
Full D3D9 Debug/Release executables link without launching the game.

Run `utils/tests/test_stored_poly.ps1` for quiet verification. Isolated full
checks use `events_project.py --stored-poly`, `check_events_project.py
--stored-poly` and `build_events_project.py --stored-poly` (plus `--release`).
Diagnostics stay under `.diagnostics/stored-poly-c-tests`, use static compiler
runtimes and verify GCC executable imports. The user's `build/` directory is
untouched. Regenerate normal project files to include the new C producer.
