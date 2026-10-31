# Complete collision-model owner conversion

The next owner after NodeName is CColModel. All of its geometry record types are
already C: bounds, spheres, boxes, lines, compressed vertices and triangle planes.
The remaining class owns allocation, release, deep assignment, plane generation,
the aligned extra-element cache link and vertex decoding.

The complete immutable original source tree is captured under
.diagnostics/col-model-c-tests/before/src, with a coherent candidate under
overlay/src. All sixteen edited production paths exactly match the coherent
candidate used for both full builds. ColModel.cpp is now ColModel.c. ColModel.h
is a self-contained C record with nine original operations and two explicit
static-lifetime operations. New ColModel_backend.h and ColModel_native.cpp retain
only six actual allocator/category/cache primitives. All callers and lifetime
boundaries are adapted and production verification is complete.

The complete original constructor, destructor, RemoveCollisionVolumes,
CalculateTrianglePlanes, RemoveTrianglePlanes, SetLinkPtr, GetLinkPtr,
GetTrianglePoint and assignment definitions are projected into C. Bounds remain
uninitialized by Init. Free order, non-owning reset behavior, unchanged metadata
and plane storage during assignment, equal-count allocation reuse and maximum
vertex-index traversal are preserved. Plane storage still reserves one extra
element, registers the live pointer before calculating geometry, and aligns the
cache-link slot using the original pointer-size expression. The public vertex
operation returns all decoded coordinates as a C value.

Current validation:

- 24 Clang/GCC x86/x64 strict producer and standalone C/C++ API compilations,
  including MASTER, compressed vertices and VU collision. VU config already
  defines COMPRESSED_COL_VECTORS, so probes define only VU_COLLISION for that
  profile to avoid a spurious command-line macro-redefinition error.
- 16 whole-producer Clang/GCC O0/O2 comparisons, four undefined-behavior sanitizer
  comparisons, and eight real MSVC x86/x64 runtime comparisons. Both whole model
  producers link actual C box, triangle and compressed-vector implementations.
- Each run has 4096 assignments and 4096 plane calculations. Default, compressed
  and VU runs have 501074 checks and 47091 balanced allocations. Custom allocator
  runs have 519506 checks and 49139 balanced allocations, 4096 push/register/pop
  sequences, and 2048 pointer relocations inside registration before geometry
  calculation reads the current slot.
- Hashes are default 373d90698c5aff5c, compressed 672073b0cd1d77a8,
  VU 0f816e877f028608 and custom allocator 27db5955bd20e276, identical across
  complete original/C producers, compilers, optimization levels and native ABIs.
- The actual separate CCollision::RemoveTrianglePlanes operation participates in
  runtime comparisons, with only its model member invocations translated to C
  calls on the after side. It uses the real C cache-list functions. No separate
  cache selection or eviction policy is modeled or claimed to be tested here.
- Eighteen actual native layout compilations compare original C++, new C++ and
  new C under x86/x64 with default, compressed and VU records. All fourteen field
  offsets match; model size remains 76/104 bytes, alignment 4/8. Actual plane
  sizes remain 20/8 bytes and compressed-vertex sizes 12/6 bytes as configured.
- Seven actual full-source native producer/primitive compilations pass. The
  preexisting custom allocator does not support x64: MemoryHeap.h explicitly
  asserts HeapBlockDesc size 0x10 and has a TODO for 64-bit pointers. Both the
  complete original producer and new native leaf reject that exact unchanged
  assertion under custom x64. native-existing-limitations.json records both
  failures separately; neither is reported as a successful compilation. The C
  model itself compiles with the custom flag under x64, and runtime owner policy
  is checked using an allocator-boundary fixture under both ABIs.
- GCC test links are static and imports are audited for unwanted GCC/MSYS DLLs.
  Diagnostic runs use a Windows-only runtime PATH and disable native error
  dialogs. No game, GUI tests or git commands are used. build/ is untouched.

Completed caller and lifetime integration:

- Pool construction/destruction in FileLoader, PedModelInfo, BaseModelInfo and
  CutsceneMgr calls explicit C functions while preserving pool storage.
- Deep assignment in Automobile and methods in Collision, ColStore, Shadows and
  Automobile uses complete C calls and bit-preserving native vector transport.
- TempColModels has 18 static collision models including its five-element array.
  Vehicle has one static rotor model. World has one local static model and one
  automatic tyre model with two returns. C static initialization registers native
  storage-address cleanup callbacks at the original startup points. The array
  registers after its final element and C destroys elements in reverse order.
  World keeps a function-local static guard for first-use initialization. Its
  automatic model is destroyed after the original output-distance write on the
  successful return and immediately before the unsuccessful return.
- PlayerInfo embeds a model. Its current implicit C++ initialization/destruction
  and deep assignment now explicitly forward embedded model operations to C.
  Its native assignment copies all existing fields in original declaration
  order around ColModel_Assign, including conditional and array members.
  Replay continues to use the parent assignment in both directions. No parent
  layout, padding view or shallow model copy is substituted.
- Thirty complete native consumer compilations pass before application and
  thirty pass from production, including both pointer widths and complete Replay
  and PlayerInfo sources. Seven production model/primitive compilations pass;
  the unchanged custom-x64 limitation is independently reproduced before/after.
- Both complete actual PlayerInfo headers yield identical compile-only native
  COFF layout constants: all sixty field offsets match, size 368/416 bytes and
  alignment 4/8. Compile-only sections avoid linking unused rouziclib header
  tables and do not launch executables for layout checks.
- Sixteen Clang/GCC O0/O2 lifetime comparisons, four sanitizer comparisons and
  eight real MSVC x86/x64 lifetime comparisons pass. Each performs 1024 parent
  assignments, observes 4515 allocator boundaries, checks 136493 conditions,
  balances 10286 allocations and verifies all twenty real static model frees
  in reverse registration order. Native storage definitions and cleanup code
  come from complete actual client prefixes. The exit observer registers early
  and checks after every original or C destructor has completed.
- Parent-copy callbacks verify the first two pointer fields are copied before
  model allocation/free operations, while subsequent fields still hold their
  previous values. Every actual parent field is checked after assignment.
  Default/custom lifetime hash is 357783df940efe8b; compressed/VU is
  4a282c0502b5b80f. The hashes match across whole original/C producers and ABIs.
- All 48 evaluated configurations select ColModel.c as C11 without object-name
  collisions. Source count is 418, or 417 for legacy Win32 RW34; paired C/C++
  basenames remain 28. All 608 complete historical source/creation projections
  pass before application and against actual production. Early snapshots that
  predate RoadBlocks_backend.h require its exact creator constant and complete
  successor projection instead of pretending an original file existed.
- Full isolated D3D9 Debug and Release builds/link pass, producing executables
  of 14333952 and 4956160 bytes. Neither game executable was launched. Every
  production source byte matches the coherent candidate used by those builds.
- Earlier compressed-vector, triangle and box final audits pass. The older
  compressed-vector native fixture freezes its exact original consumer only
  after proving the complete current C model successor and literal GetTrianglePoint
  method identity. Its original float/compressed/VU runtime hashes still pass.
  The preceding NodeName final audit also accepts only exact successor proofs.
- The node_name migration endpoint wraps the model conversion. stats_successor
  records the explicit ColModel.cpp -> ColModel.c rename. apply_col_model.py gated
  all staged proofs before changing production. The archived NodeName project
  uses its private stage projection; archived byte digests allow only explicit
  complete later transformations.

col_model_stage_audit.py --production --integration reports applied-verified with
no remaining model work. The overall project conversion is still active. All
tool/compiler handles are terminal. Do not regenerate immutable overlays or
fixtures while a later compiler still uses them. No git or GUI tests were used;
build/ remains untouched.
