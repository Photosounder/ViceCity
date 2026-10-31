# Animation plugin conversion in progress

Production RpAnimBlend.cpp, its public header and all callers remain unchanged.
The coherent complete candidate is under .diagnostics/anim-plugin-c-tests/overlay,
with a full immutable before/src snapshot. The completed frame callback stage is
already applied and verified; its source tree and all tool handles are stable.

anim_plugin_migration.py projects the WHOLE original plugin owner into C11:
35 complete operations (21 actual public operations and 14 internal lifecycle,
hierarchy and lookup callbacks), plugin state ClumpOffset, pFrameDataFound global,
list removal/query policy, skin/non-skin frame initialization, frame/bone search,
keyframe refresh and the complete playback update pipeline. Allocation, loop
order, cached next links, strict blend comparisons, output parameters, 11-entry
node collection limit, normalization expressions and compression/render branches
are retained. No partial algorithm subset is intended as the final conversion.

The 21-operation public API now uses C linkage and opaque SDK clump handles. The
masked next/first overloads have explicit WithMask names. World and weapon calls
that formerly depended on the render default argument now pass true explicitly.
Ten source/header paths are changed; eight native caller sources receive only
the diagnosed overload/default call edits. A legacy FillFrameArrayCallBack public
declaration has no definition or caller anywhere in src; the actual internal
callback is FillFrameArrayCBnonskin. The C API lists the implemented operations.

RPANIMBLENDCLUMPDATA addresses the genuine SDK-registered slot through a C inline
helper using ClumpOffset and byte pointer arithmetic, just as the old SDK macro
did. Plugin construction still uses offsetInObject, not the global offset, and
the original unsupported copy callback returns null. Existing association owner
interfaces for plugin data/frame/bone lookup become direct C forwarders.

Native RpAnimBlend_native.cpp retains the original eight allocator/memory-category
leaves and sixteen SDK/external primitives. SDK child enumeration adapts callback
pointer types through one stack context and trampoline; recursive counting/filling
remain in C. Native hierarchy IDs, interpolation frame lookup and visibility/name
metadata use actual SDK objects. SkinGetBonePositionsToTable is an existing
separate external owner; its native call transports initialized input and output
coordinates through actual RwV3d arrays using component memcpy. It preserves the
original 64-entry zeroed table and each coordinate bit. No SDK record layout is
invented in C. A native static_assert checks the original plugin ID 0x0253F2FD
against MAKECHUNKID(rwVENDORID_ROCKSTAR,0xFD).

Passing staged evidence:

- Complete producer compiles as strict C11 with Clang and GCC, x86/x64, default,
  ANIM_COMPRESSION, MASTER and USE_CUSTOM_ALLOCATOR compilation profiles.
- The public API and backend headers compile standalone in strict C11 and C++14
  without common.h or SDK type definitions. There are 24 total strict producer/API
  object compilations. This does not establish a full custom-heap game build.
- Twenty complete real-source MSVC compilations pass: C producer, separate native
  leaf unit and all eight changed caller sources under both actual native ABIs.
- anim_plugin_stage_audit.py verifies all exact complete projections, 35 original
  operations, every C declaration, preserved policy anchors, unchanged production
  byte digests, all native leaves and the compilation evidence counts.

Complete original/C runtime comparisons now pass:

- Both whole plugin producers compile, rather than extracting selected methods.
  The before side retains all original C++ method bodies and native leaves. The
  after side uses the actual full C producer and separately projected native unit.
- Both link genuine C association, node, sequence, hierarchy, list, clump,
  keyframe, quaternion, General, bone and complete frame callback implementations.
  SDK registration/enumeration/metadata and allocation are controlled at actual
  native boundaries. The separate manager operation records calls and invokes
  real hierarchy uncompression when requested; this plugin fixture does not
  reproduce or claim to test the manager's separate LRU cache policy.
- 16 Clang/GCC O0/O2 runs cover fixed/classic and ANIM_COMPRESSION combinations;
  four undefined-behavior sanitizer runs and eight actual MSVC x86/x64 runtime
  comparisons also pass. The public after caller is independently compiled C11.
- Each run compares 4096 association-query scenarios, 128 initialized frame
  hierarchies spanning every 1..64 bone/frame count, 256 masked deletion cases,
  and 1536 complete playback updates over 512 initialized clumps.
- Every run has 62499 checks, 2 registrations, 5320 SDK child enumerations and
  5387 balanced aligned frame/node allocations. Real blend deletion invokes 256
  callbacks before releasing their actual nodes. Real finish-time updates invoke
  1153 callbacks after clearing the finish discriminator; callbacks mutate live
  playback state. Normal hash 5a59951492017532; compression 056368dc66581caf.
- Registration failure and success preserve the original result and global slot.
  Actual constructor/destructor/copy callbacks execute, and construction is tested
  at offsets that differ from the registered global slot. Unsupported copy stays
  null. Actual clump slot pointers remain intact outside each provided offset.
- Recursive non-skin enumeration and frame-array indexing use real C policy.
  Skin setup verifies all copied coordinate bits, native pose handles, bone tags,
  last-match duplicate names/tags, unsigned-to-signed tag transport and ped-node
  indexing. Real C association binding crosses all three formerly native owner
  interfaces for plugin data and frame/bone lookup.
- Queries cover mask zero, negative/high flag patterns, strict blend ties, main/
  secondary outputs, partial/N-th queries and initialized versus null owners.
  Masked deletion checks actual survivor order and frees every real node array.
- A native frame-name getter reenters another actual lookup. The original shared
  global search result is retained even when the outer lookup has no match; this
  preserves the original reentrant behavior rather than introducing new state.
- Complete playback covers partial/movement weights, the eleven-node collection
  cap, unloaded hierarchies, deletion, finish callbacks, render/offscreen refresh,
  packed dispatch and native object-update ordering. Native velocity storage
  dimensions remain valid for each original callback contract.
- Six actual MSVC C/C++ layout compilations prove ClumpOffset's 32-bit width,
  clump/frame/association/dispatch storage sizes and offsets, and both native/C
  vector widths. The API stays independent of SDK class layouts.
- anim_plugin_stage_audit.py now checks all exact complete projections, original
  operations and policy anchors, runtime result counts/hashes and unchanged
  production byte digests. It reports staged-incomplete; production is unchanged.

Integration and production application are complete:

- All 48 evaluated configurations compile the plugin algorithms as C11. The
  coherent staged D3D9 Debug and Release executables fully build and link, with
  sizes 14329856 and 4952576 bytes respectively. Neither executable was launched.
- All 360 complete historical source projections pass, both before application
  and against actual production. Every production source byte matches the
  coherent candidate tree used for both full links. Twenty actual production
  sources compile under MSVC x86/x64, including all eight changed consumers.
- The frame_update endpoint now wraps the complete plugin projection and its
  project metadata. stats_successor records the explicit producer rename.
  Source count is 416, or 415 for legacy Win32 RW34; paired basenames remain 28.
- Earlier association, sequence, clump and frame fixtures prepare successfully
  from production and preserve their established runtime hashes. Their native
  historical consumer schemas remain frozen under complete successor proofs.
  Six custom-allocator sequence probes and six production frame/SDK probes pass.
- Earlier association, sequence, clump and frame callback final audits pass
  against the complete C plugin successor. Existing allocator leaves remain
  unchanged in the separately verified native producer. Association host aliases
  now forward directly to C algorithms, with no native algorithm bridge.
- Diagnosis found an older frame migration would inject native adapters again
  into the complete C plugin. Its completed-owner guard now preserves direct
  C frame fields. The clump fixture and frame source probe resolve only explicit
  completed renames. Archived project proof uses its private stage projection;
  archived whole-source digests allow only exact explicit successor transforms.
- anim_plugin_stage_audit.py --production --integration reports applied-verified.
  The full project is still being converted; this completes the plugin owner.

All current tool/compiler handles are terminal. Do not regenerate any immutable
source tree or fixture while a compiler using it is alive. Use .diagnostics,
quiet static diagnostics, GCC/MSYS import audits and Windows-only runtime paths.
No git or GUI tests were used.
