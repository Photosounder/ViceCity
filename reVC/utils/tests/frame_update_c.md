# Frame callbacks converted to C11

The complete callback conversion is applied to production. Its exact compiled
source tree is retained under .diagnostics/frame-update-c-tests/overlay with a
coherent immutable before/src snapshot. Bone mapping production conversion is already complete;
finalize_bones.py --production and finalize_anim_manager.py pass.

frame_update_migration.py projects all nine complete callback algorithms into
FrameUpdate.c, including normal/skinned accumulation, both velocity modes,
offscreen delegation and both branches of compressed callbacks. The full loops,
flag tests, FIX_BUGS sign policy, native output store order, original 2D height
adjustment and update.nodes pointer advancement remain intact. Local native
vector constructors/operators become ordered C component operations. Five small
private C helpers implement vector output references, addition, subtraction and
storage; no blending or velocity-extraction policy lives in C++.

AnimBlendFrameUpdate.h owns the original int foobar / nodes[16] dispatch record,
all nine callback prototypes and the gpAnimBlendClump global with C linkage.
RpAnimBlend.h includes this C API and retains its other native SDK/plugin APIs.
The global is defined by the C callback owner. Real native ABI checks prove the
dispatch record remains 68 bytes on x86 and 136 on x64, with unchanged offsets.

FrameUpdate_native.cpp has nine individual native primitives: frame matrix
lookup, SDK matrix identity/update, and borrowing actual matrix rotation/position,
pose quaternion/translation and native 2D/3D velocity field pointers. These return
live field references into actual SDK/native objects; they do not construct fake
SDK record layouts or snapshot output values. Scalars remain read/written by C
at the original statement positions. Ordinary and compressed node interpolation,
association predicates, sequence flags and quaternion operations use their real
existing C implementations.

Passing staged evidence:

- Full strict C11 candidate compilation and a sixteen-run Clang/GCC O0/O2
  original/C matrix across fixed/classic and compression profiles.
- Every run executes 18432 actual frame callbacks with 36865 checks, through an
  independent C11 dispatcher, with real association, blend node, sequence,
  keyframe, quaternion, list and clump C producers in both phases.
- Fixed hash f6aeff59d0708f9f; classic hash 1e5a375672fe99b3. Compression profiles
  keep those respective hashes. Inputs cover ignore flags, partial weights,
  velocity flags and null velocity pointers, looping, native pose/matrix writes,
  quaternion signs, rotation-only and translation sequences, and pointer advance.
- Four undefined-behavior sanitizer runs and eight actual MSVC x86/x64 runtime
  comparisons produce the same hashes. C/C++ field-reference struct returns and
  global linkage are exercised across the actual native ABIs.
- Six real-source MSVC compilations cover complete FrameUpdate.c, the new native
  primitive unit and the entire actual RpAnimBlend.cpp consumer in both widths.
- Six native C/C++ layout compilations check original/new dispatch size, offsets,
  sixteen-node array extent, and C field-reference descriptor sizes.
- frame_update_stage_audit.py proves complete deterministic source projections,
  all nine definitions, exact native leaves, original node-advance statement
  count, runtime results and that production remains unchanged.

All current compiler/tool handles are terminal. Never regenerate fixture or
overlay files while a compiler using them is alive. Runtime scripts accept
--prepared to avoid rewriting fixture sources. Quiet static binaries run with
only Windows system paths, reject GCC/MSYS runtime DLL imports and suppress
native error windows.

Final production integration passes:

- Live SDK mutation cases add 18432 comparisons per run and 55297 checks. SDK
  frame-matrix lookup changes live frame flags, reset positions, dispatch weights
  and optionally gpAnimBlendClump; identity changes the translation guard and
  current output owner; final matrix updates mutate reset position. Both original
  and C callbacks observe the same current fields and write the same real native
  velocity storage. Fixed hash faf0470ab17bca14, classic 0f8ca137f0aadcf4.
- Mutation matrix: 16 Clang/GCC optimizer/profile runs, four UBSan runs and eight
  MSVC x86/x64 runtime runs all match. Ordinary 16+4+8 runs retain their original
  hashes and counts. All SDK mutation hooks are conditional fixture-only code.
- All 48 evaluated build configurations compile the callback producer as C11,
  retain 28 paired object names, and have 415 sources (414 in Win32 RW34).
- Full D3D9 Debug and Release links pass with 14326272/4951552 byte executables.
  No game was launched. The final audit compares every production source/header
  byte with the entire staged tree used by those builds.
- All 70 complete historical callback/header projections pass before and after
  application. The bones migration endpoint wraps complete frame ownership for
  both source and project transformations. stats_successor records the explicit
  FrameUpdate.cpp -> FrameUpdate.c producer rename.
- Six real production MSVC compilations pass for the complete C producer, its
  separate native primitive unit and actual RpAnimBlend.cpp consumer in x86/x64.
- Earlier association, sequence, frame-record and blend-node fixture preparation
  and smoke comparisons pass with their unchanged original hashes. The association
  fixture freezes the exact previous native callback consumer only after proving
  its complete current C successor; other historical fixtures retain their exact
  earlier schemas. Renamed real producers resolve through current_path.
- The quaternion finalizer passes after two real legacy-idempotence defects were
  diagnosed and fixed: keyframe migration must not apply KeyFrame_FromNative to
  an already-C manager translation, and frame-record migration must not insert
  a native inline adapter into the completed C callbacks' opaque pose handle.
  Guards identify complete C owners; earlier native-source projections still run
  in full. No production algorithm was changed by these tooling fixes.
- Association, group, manager and bone finalizers pass after the frame conversion.
  Archived bone project proofs retain their private stage projection, and older
  complete source proofs accept only exact byte matches or explicitly verified
  later owner conversions.
- finalize_frame_update.py reports applied-verified with no remaining frame-stage
  work. All known compiler/build/tool handles are terminal. The overall main
  project conversion remains active; RpAnimBlend.cpp and other C++ owners remain.
