# Animation sequences in C

`AnimBlendSequence.c` contains the complete sequence owner: initialization,
destruction, fixed-width naming, frame allocation, actual keyframe getters,
translation flags, bone tags, quaternion sign correction, compression,
uncompression, unpacked data removal and optional heap relocation. All thirteen
operations have a C11 interface. Animation loading, hierarchy processing,
cutscenes, pedestrian AI and all frame callbacks use it.

There are no sequence subclasses or external virtual dispatch users. The unused
virtual destructor and its hidden pointer are removed rather than represented by
a dummy pointer. The real record shrinks from 48 to 44 bytes on x86 and 64 to 56
bytes on x64. Its name remains uninitialized until SetName, whose original 24-byte
strncpy behavior is retained. Native array allocation, construction and destruction
now use the complete C record and explicit Init/Destroy calls. Blend nodes read
the complete C sequence directly; the obsolete native sequence field interface
has been removed.

The six native memory primitives preserve SDK allocation/free, memory category
push/pop, live pointer registration and optional heap relocation. All storage
decisions remain in C. In particular, packed SetNumFrames still allocates using
the original unpacked sizes; it still accumulates flags and does not implicitly
free a previous buffer. Uncompress retains its reciprocal multiplication for time,
and registers the new live pointer before freeing packed storage. Relocation keeps
the original first-buffer priority.

Independent original/C executables compare 93,200 states and 202,786 invariants
per ordinary profile, plus 512 relocation cases in the custom allocator profile.
They cover every signed packed value, 4,096 storage roundtrips, 4,096 partial
initialization/name cases, 1,024 allocation boundary cases, all thirteen operations
and 18,432 cases across all nine actual frame callbacks. Allocation events compare
the live owner fields and operation order; native addresses and removed vtable
bytes are excluded from observations. Original objects are not read after their
lifetimes end. Borrowed keyframes are detached before original automatic cleanup.

Clang/GCC O0/O2, UBSan and native MSVC x86/x64 cover fixed, classic and custom heap
policies. All diagnostics are quiet and statically linked where applicable. Native
checks compile 66 complete units and six x86 custom allocator units before and
after application. Native probes verify the new record layout and the unchanged
parent layouts. All 48 generated configurations keep distinct object names, and
isolated D3D9 Debug/Release links run without launching the game. Historical source
audits cover 322 explicit projections.

Earlier frame, node and keyframe fixtures retain their exact historical sequence
schema, proved through their private projections. The sequence fixture tests the
current complete C owner. Run `utils/tests/test_anim_sequence.ps1` after generating
the isolated project and full build reports. Never regenerate a fixture or overlay
while a compiler is reading it.
