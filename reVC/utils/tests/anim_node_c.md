# C animation blend nodes

`AnimBlendNode.c` owns the complete node record and all thirteen former methods:
partial initialization, ordinary/packed updates, ordinary/packed frame advance,
time search, packed setup, both angular-delta calculations, both current-translation
queries and both endpoint-translation queries. The C++ class and member methods
are removed. Native association, frame and render callbacks use the public C
functions. The record stays 28 bytes on x86 and 40 bytes on x64, with unchanged
angles, frame indices, remaining time and native owner pointers.

Two native interfaces expose actual sequence and association field addresses.
They perform no animation algorithms and read no field values. The C policy
loads fields at their original access points, retaining count-only early exits,
unused storage pointers, stopped-association timesteps and zero-blend queries
without a sequence. There is no cached native snapshot or synthetic class view.
The original association flag enum is shared through a C header. Keyframe flags
have C constants, and the remaining sequence class uses aliases to those values.

Initialization still changes only frame A/B, remaining time, sequence and
association. It preserves both angular fields and native alignment padding.
Frame advance retains inclusive timing tests, repeated wrapping, non-repeating
end behavior, early return state and angular recalculation order. Time search
retains its original preincrement and subtraction order. Compressed angular
calculation keeps its negative-hemisphere correction and writes the changed
packed keyframe before recomputing its dot product. No additional clamps or
termination rules are introduced.

Translation operations retain complete vector temporaries, left/right scalar
multiplication order, immediate XYZ output and separate blend scaling. Quaternion
operations use the existing C producer. Public output references address actual
native vector fields; a native vector class is never overlaid on C coordinates.
The C node header retains the old sequence include for native C++ consumers while
providing opaque native owner pointers to C callers.

The strict C11 fixture compares the entire original implementation against C over
71,691 states and 208,917 invariant checks. It includes 4,096 partial initializers,
49,152 direct method cases, original no-write/default/static lifetimes, ordinary
copies, empty and one-frame sequences, zero-duration frames, repeats, ending,
negative and nonfinite weights, packed/unpacked data and actual output guards.
Nine lazy-read cases exercise null unused owners and storage, including an
uninitialized timestep that must remain unread on a stopped association.

All nine complete frame callbacks execute across 18,432 skinned/non-skinned,
packed/unpacked, velocity, partial-weight, loop and ignore-channel scenarios in
fixed and classic profiles. Actual production field-interface bodies compile
against native fixture owners. Native compiler-emitted COFF constants separately
compare association, sequence and update-data parent sizes/offsets on x86/x64.
All 48 staged and 48 production source/layout checks pass in float, compressed
and VU configurations. Clang/GCC O0/O2, UBSan and MSVC x86/x64 match their original
implementations. Historical checks cover 234 earlier source versions and the C
keyframe-header introduction across 26 previous stages.

All 48 evaluated project configurations select C11 and preserve 28 distinct
C/C++ basename pairs. Full D3D9 Debug/Release executables link without launching
the game. The earlier keyframe and quaternion fixtures retain their original
node/keyframe schemas; the node fixture tests the current complete consumers.
Historical path resolution recognizes explicit producer renames, and the older
keyframe migration leaves completed C node code free of native vector adapters.

Run `utils/tests/test_anim_node.ps1` for quiet verification. Full isolated checks
use `events_project.py --anim-node`, `check_events_project.py --anim-node` and
`build_events_project.py --anim-node` (plus `--release`). Diagnostics remain in
`.diagnostics/anim-node-c-tests`, use static compiler runtimes, check GCC imports
and leave the user's `build/` directory untouched. Regenerate normal project
files for the `AnimBlendNode.cpp` to `AnimBlendNode.c` rename.
