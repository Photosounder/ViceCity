# Animation frame record in C

`AnimBlendFrameData.h` owns the complete record in C11: an eight-bit flag, three
reset-position floats, the original pointer union and a signed node ID. The six
scoped constants become `ANIM_FRAME_*` constants with unchanged values. Native
sizes remain 24 bytes on x86 and 32 bytes on x64. Default construction still writes
nothing, copies remain trivial and static storage stays zero initialized.

Native SDK pointers are stored as opaque pointers in the common C representation.
Two inline native adapters recover the original SDK pointers, so animation and IK
writes reach the original live matrix or pose. The position adapter transfers the
three coordinate representations without arithmetic. No animation algorithm moves
into an adapter. Clump allocation now uses `sizeof(AnimBlendFrameData)` directly;
traversal reads the live count and frame pointer around each callback. The native
frame-size query has been removed.

The independent C11 caller and original/C executables compare 25,612 states and
80,209 invariants per run, including arbitrary coordinate bits, value copies,
self-borrowed position input, ten allocation size boundaries, four actual frame
initialization/search callbacks, torso IK, 2,048 mutating traversals and 18,432
executions across all nine actual animation callbacks. Fixtures select the real
active frame and velocity union members. Ordinary copies exclude unspecified
padding from observations.

Clang and GCC at O0/O2, UBSan, and native MSVC x86/x64 compare both fixed and classic
policies. All diagnostic executables are quiet; GCC imports are checked and runs
use a Windows-only runtime PATH. Native production checks compile 66 units across
three data profiles and compare parent sizes and offsets. All 48 generated build
configurations retain distinct object names, and isolated D3D9 Debug and Release
builds link without launching the game. Historical audits cover 254 explicit
source projections, including the two headers introduced by the prior clump stage.

The earlier node and clump fixtures keep their exact historical frame schema,
validated against their private migration projections. The frame fixture tests the
current schema and producer. These checks preserve historical reference behavior
without adding production compatibility wrappers.

Run `utils/tests/test_anim_frame.ps1` from the project root after generating the
isolated project and build reports. Do not regenerate fixtures or overlays while
their compilers are running.
