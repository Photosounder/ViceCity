# Association groups and animation blocks in C

AnimBlendAssocGroup.c contains all nine original group operations, both selection
helpers and explicit static lifetime registration. AnimBlock.h contains the full
C11 block record formerly declared in AnimManager.h. Native layouts remain 20/32
bytes for groups and 32 bytes for blocks. Model selection, copying, initialization,
destruction and both creation paths remain entirely in C. Thirteen native leaves
provide only metadata, manager and SDK primitives and prefix comparison.

The conversion preserves the surprising original details: paired-digit and prefix
matching, ASCII versus locale character functions, CSplay player substitution,
ordered model scans and repeated native getters, local animBlock shadowing in the
model-based overload, member block assignment in the clump overload, untouched
count when clearing null storage, and full association count with missing model
slots. Missing slots retain their original partial association initialization.

Manager arrays now use explicit C initialization and destruction. The cutscene
client retains its original native dynamic initialization timing, now invoking
AnimGroup_InitStaticCutscene, which owns the initialization and atexit cleanup in
C. It registers the actual static record address and returns its initial value.
No new C++ owner or RAII wrapper is introduced. The remaining native client dynamic
initializer will be removed when CCutsceneMgr itself is converted.

Original/C executables compare 19,456 cases and 44,399 invariants: 2,048 lookup/
ownership cases, 16,384 defined ASCII character pairs and 1,024 creation/model
selection cases. Actual C association, node, sequence, quaternion, keyframe and
link producers run in both phases. Native primitive events normalize addresses
and verify ordering, classification, skin callbacks and clump release. A separate
child process populates a static group and verifies groupId=-1 before main's work
and actual aligned node release after main returns. Original automatic C++ exit
cleanup and explicit C exit cleanup produce identical output.

Clang/GCC O0/O2, UBSan and native MSVC x86/x64 compare fixed/classic and compression
profiles. Native MSVC additionally uses ASCII_STRCMP as the real Windows build
does. All runs report cases=19456 checks=44399 hash=93ec8d46a9c54073 | exit=passed.
Diagnostic executables are quiet and GCC imports exclude libgcc/libstdc++/MSYS
runtime DLL dependencies. The native source suite compiles 36 units across the
three data profiles before and after applying. All 48 generated configurations
retain C11 metadata and 28 paired object names.

Historical audits prove 193 explicit projections, including the complete new block
record introduction. Full isolated D3D9 Debug/Release links and a final source/
lifetime/selection audit complete validation. The game is never launched. Run
test_anim_group.ps1 after generating the isolated build reports. Do not regenerate
any fixture or overlay while its compiler handle is still live.
