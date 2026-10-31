# Animation manager converted to C11

The complete conversion is applied to production. The exact applied source tree
is retained under .diagnostics/anim-manager-c-tests/overlay, with a coherent
immutable before/src tree. anim_manager_migration.py now projects all 28 operations into C,
including complete IFP parsing, cache ownership, reference counting, association
blending and group creation. All 35 changed source/header paths are applied. The
separate AnimManager_native.cpp contains only preexisting SDK model/clump leaves
and five primitive boundaries: stream open/read/close, streaming removal and name
comparison. Manager aliases used by earlier C owners now forward directly in C.

Run anim_manager_prepare.py only when no compiler is using the overlay. It checks
partially saved originals before completing the snapshot and inventories actual
consumer overload types. Integer/name overloads have explicit C names. The fight
data parser requests associations by name; other diagnosed consumers use IDs.
Native stream consumers retain all three explicit arguments. Source edits carry
project markers and comments, and generated files use LF line endings.

All original descriptor/table initializer contents, ordering, profile branches and
special counts remain intact. MI_COP is an enum constant in ModelIds.h, so static
C initialization preserves the original table expressions. Descriptor sizes are
8 and 24/40 bytes. C translation records replace two loader CVector values, with
field references passed directly to the existing compressed keyframe functions.
The loader retains KRTS/KRT0/KR00 reads, scalar arithmetic order, zero-frame paths,
original ROUNDSIZE statements (including dgan.size when reading cpan), compression
selection and time conversion. The original project assertion handler and MASTER
expression evaluation policy remain in the C kernel.

Current passing evidence:

- anim_manager_strict_c.py: 16 strict C11 object compilations with Clang/GCC,
  32/64-bit, default/compression/MASTER/handheld profiles. GTA_HANDHELD exercises
  the table branch without PC_PLAYER_CONTROLS; config.h owns that latter define.
- anim_manager_native_sources.py: 12 real MSVC compilations of the entire C
  manager and native leaf unit, 32/64-bit, default/compression/MASTER profiles.
- anim_manager_native_sources.py --consumers: 70 complete real-source native
  compilations of every changed producer and consumer in both pointer widths.
- anim_manager_policy.py: four original/C runtime comparisons, Clang/GCC at O0
  and O2. Each performs 32768 cache operations and 852292 checks; all produce
  hash 2c2f613b635029d2, 6514 decompressions, 4853 evictions and 440 packed-time
  requests. Tests cover registration, name folding, reference removal boundaries,
  actual cache membership/link order, capacity eviction, promotion, removal and
  keep-compressed lazy duration behavior. The test compiles entire selected
  methods from the original and actual staged producers with real C records and
  LinkList operations. Controlled hierarchy/streaming leaves observe manager
  policy; this is not evidence of complete downstream playback or loading.
- anim_manager_stage_audit.py: exact projections, production unchanged hashes,
  C-only kernel syntax, all 28 definitions and the evidence counts above pass.

Policy test binaries link statically, GCC imports exclude libgcc/libstdc++/
libwinpthread/MSYS runtime DLLs, and executions use only Windows system paths.
SetErrorMode suppresses native error windows. Diagnostics do not recreate build/.

Additional passing runtime and ABI evidence:

- anim_manager_loader.py: 16 original/C matrix runs plus four undefined-behavior
  sanitizer runs. Each loads 1024 independent streams and observes 69300 reads,
  with 128 filename stream open/close pairs. Cases cover all three keyframe tags,
  chunk rounding, bone tags, zero-frame nodes, exclusion lists, pre-registration,
  exact packed/unpacked fields, quaternion flips, time differences and actual
  eviction beyond the 25-entry cache using real hierarchy/sequence dependencies.
  Non-compression runs have 105893 checks, 6447 balanced keyframe allocations and
  hash 0605650ecf697958; compression runs have 143343 checks, 12961 allocations and
  hash b9d4c85cf4948a68. MASTER profiles preserve the same outputs.
- anim_manager_loader_native.py: eight matching original/C MSVC runtime runs,
  32/64-bit and four default/compression/MASTER combinations, with the same hashes.
- anim_manager_full.py --matrix: 16 original/C comparisons of the COMPLETE manager
  producers (no method extraction). The original producer retains every original
  method body and native host definition; the C producer is the actual staged
  complete source and its separately projected native leaves. Both use real C
  hierarchy, sequence, keyframe, quaternion, General, group, association, blend
  node, list and clump implementations. SDK and external allocation/streaming
  primitives are controlled fixture boundaries. The fixture encodes all 377
  distinct names from original tables, creates all 33 ped association groups,
  and performs 4096 blending scenarios covering additions, synchronization,
  existing IDs, running/end states, movement and partial/full-body fade choices.
  It compares all initialized playback fields, real node relationships, active
  list order, native creation/skin cleanup/destruction order, unload/reload,
  reference-triggered streaming reentry and complete shutdown ownership.
- Full producer matrix results: non-compression 173267 checks and hash
  38354bbdd2a1ce0d; compression 211012 checks and hash 56c38a3a648f7abb. Both have
  66 model instances, 33 skin removals, 36 streaming removals and 17720 balanced
  aligned node allocations. A safe failed registration precondition verifies
  that the original project assertion reporter can return and execution can
  continue. MASTER cases require zero assertion reports.
- anim_manager_full.py --sanitized: four matching full producer UBSan runs.
- anim_manager_loader_native.py --full: eight matching complete producer MSVC
  runs, 32/64-bit and four profiles, with the same full-producer hashes.
- anim_manager_layouts.py: six native C/C++ layout compilations prove every
  descriptor offset and size, plus the block layout, under both MSVC ABIs.
- Updated anim_manager_stage_audit.py verifies every count/result, the exact
  complete projection, and that all production sources still match the snapshot.

The loader-focused comparison extracts whole loader-related methods without
altering bodies; the full producer comparisons additionally compile every method
and resolve all native leaves. Keep both: generated varied streams provide broad
parser coverage, while full producers exercise cross-owner lifecycle behavior.
Fixtures and overlays must never be regenerated while a compiler using them is
alive. All latest runtime, native compilation, history and build tool handles have completed.

Final production integration passes:

- 48 evaluated MSBuild configurations compile AnimManager.c as C11, retain 28
  paired C/C++ object basenames, and contain 414 sources (413 in Win32 RW34).
  AnimManager_native.cpp has a distinct basename and holds only native primitives.
- The isolated D3D9 Debug and Release builds both pass and link: 14323200 and
  4950528 byte executables. No game or GUI was launched. All 414 project producers
  point into the coherent staged tree; the final production audit proves every
  staged source/header byte equals production, so these links cover the exact
  implementation that was applied. The project projection is idempotent.
- 1155 complete historical source projections pass both before and after apply,
  spanning all 35 changed paths across 33 earlier conversion stages. The explicit
  successor map now resolves AnimManager.cpp to AnimManager.c. The group migration
  endpoint wraps the manager migration for both source and project projections.
- All 70 real production consumer/producer compilations pass in MSVC x86/x64.
- Earlier group and association finalizers pass unchanged behavior hashes. The
  hierarchy finalizer also passes after recompiling its 18 optional compression
  and custom-allocator source probes against the actual new manager calls.
  Its pre-existing full custom-heap Game.cpp limitation is unchanged: the focused
  probe omits only the identical DrasticTidyUpMemory function that already fails
  from unavailable GTA3 level/collision APIs. This does not prove a full custom-
  heap game build. Both actual changed memory-move callers compile in the probe.
- Historical verification reuses a successful source proof only when the saved
  digest and the actual current byte digest match, avoiding repeated expensive
  projections without permitting stale proofs after a source edit.
- finalize_anim_manager.py / anim_manager_stage_audit.py --production --integration
  reports applied-verified with no remaining manager-stage work. The project-wide
  C conversion goal remains active; other C++ owners and SDK boundaries remain.
