# Animation associations in C

AnimBlendAssociation.c owns all 28 operations, including both construction
policies, clump binding, copying, scalar queries, playback, callback setters,
completion callbacks and self-destruction. All native consumers use the C API.
The four external boundaries retain SDK clump/frame/bone lookup and the separate
animation manager operation; they contain no association algorithms. Aligned
allocation reuses the existing native primitive. Nodes read actual C association
fields and no native association-field interface remains. Its old header remains
only as a C-compatible include used by explicit historical projections.

Fixed layouts stay 56 bytes on x86 and 88 on x64. Classic probes verify removal of
unused virtual dispatch: 60 to 56 bytes on x86, 96 to 88 bytes on x64, with link
offset moving from 4/8 to zero. No subclasses or binary association serialization
users were found. Array allocations, explicit lifetime calls and link recovery all
use the complete current C record.

Independent original/C callers exercise every operation. Constructors' originally
indeterminate fields are initialized before observation. Copy and InitFrom retain
live alias behavior rather than snapshotting the source. SetCurrentTime retains
wrapping order, the manager boundary and conditional packed node setup. Completion
clears its callback discriminator before callback invocation. Delete callbacks may
mutate live state; node storage is freed before unlink, and UpdateBlend releases
the actual owner before returning false. Tests never read a destroyed owner.

The suite compares 73,734 states and 292,889 checks, plus 18,432 cases across all
nine actual FrameUpdate.cpp callbacks using real association records and the native
int-plus-node-array update layout. It covers every flag pattern, clump binding,
allocated nodes, distinct/self copies, both callback setters, current-time/start/
synchronization setup, link recovery, live callbacks and self-destruction. Original
native methods run in the reference executable; production C node, sequence,
quaternion, keyframe and link implementations run in both phases. Positive
allocation cases fit the fixture's storage; negative count cases verify the
original argument conversion without entering a positive initialization loop.

Clang and GCC O0/O2, UBSan and MSVC x86/x64 cover fixed, classic and compression
policies. Hashes are d64e60205cefbc01 (fixed), 4c5ca17a26158fa0 (classic),
81e9f13958682075 (compression), d5bd9506ff84eb5c (compression/classic). Native
checks compile 132 units across three data profiles and four classic layout probes
before and after applying. Diagnostics are quiet; GCC linkage is static and its DLL
imports are checked before execution with a Windows-only runtime PATH.

All 48 generated configurations preserve C11 metadata and 28 paired object names.
Full isolated D3D9 Debug/Release builds link without launching the game. Historical
source audits cover 811 explicit projections, including the old node interface's
introduction. Older fixtures select their exact historical association schema,
proved through explicit successor projections; their original results are retained.

Run test_anim_association.ps1 after generating isolated build reports. Wait until
all compiler handles are terminal before regenerating fixtures or overlays. The
broader project conversion remains active after this owner is complete.
