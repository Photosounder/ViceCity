The remaining user-display owner is now C11. CPlaceName and CCurrentVehicle are
plain records with canonical PlaceName_state and CurrentVehicle_state instances;
the empty static CUserDisplay class is removed. Nine complete operations live in
UserDisplay.c. Both C++ constructors are removed: their original field-only Init
bodies are retained as C operations, while static C zero initialization supplies
the same original startup field values and padding. The Game and Frontend callers
use the C APIs. Existing timer and pager APIs retain their original call order.

User.cpp contains twelve typed native boundaries: one captured player position,
the two HUD name setters, pointers to two existing HUD history/state slots, live
pedestrian/vehicle/taxi queries, native model index and game-name access, native
text lookup and the replay predicate. All zone-transition decisions, counters,
display selection, current-vehicle storage, field resets and update coordination
are in C. Existing C zone APIs are called directly. The opaque vehicle pointer
crosses no representation cast in C; native inheritance/downcasts remain typed
inside C++. The native position is copied once before both zone queries.

Original native probes established place-record sizes of 12/24 bytes on x86/x64
and vehicle-record sizes of 4/8 bytes. All field offsets/types, alignments and
nine corresponding method/API signatures are compared after conversion. Records
retain standard layout, trivial copying and trivial destruction while becoming
trivially default constructible. Both canonical objects are checked byte by byte
against their original constructor-equivalent zero state before any test setup.
The uniquely named C source is explicitly registered as C11 in Visual Studio;
the existing CMake/Premake source globs include it.

Twenty portable original/C comparisons cover Clang/GCC O0/O2 with default, plain,
MASTER, unfixed and GL profiles. Nine MSVC O2/LTCG comparisons cover default, GL,
MASTER and unfixed on both architectures plus Win32 vanilla. Five Clang O1
comparisons trap undefined behavior and float-cast overflow. Every pair matches.
The randomized driver sequences every draw explicitly, and native execution uses
the same freshly compiled executable filenames as native linking.

Default runs observe 7,609 complete states, 2,745 captured positions, 5,490 zone
queries, 2,884 translations, 3,416 zone HUD writes, 3,976 pedestrian checks, 475
model/text queries, 1,918 vehicle HUD writes, 674 replay checks, 712 calls to each
subordinate Init and 541 calls to each subordinate Process, plus twelve mutating
callbacks. Unfixed runs observe 7,511 states and eleven mutating callbacks; their
root replay gate is absent. Each run has over 83,000 independent assertions. Hash
differences by native pointer size include the original retained record padding:

    x64/default: ec5799cc6483461e
    x64/unfixed: c1cec407d39cf58e
    x86/default: 27af060aa7d4695e
    x86/unfixed: 32acfb8000580376

Win32 vanilla follows the unfixed result. The unfixed original Display method has
an uninitialized text pointer when both zone slots are null; comparisons retain
the original conditional source and restrict unfixed inputs to its defined cases.
They do not claim equivalent behavior for that original undefined path.

The oracle compiles all nine unmodified original bodies, both constructors and
the original record/class declarations, exposing stored fields only for direct
observation. Both versions enter through separate genuine C11 callers; original
callers use opaque pointers. The converted version compiles the actual production
C file and all twelve complete native bindings. Native fixtures use real project
common/types, config, vector and zone declarations. They compile the complete
original player-position operation and HUD pointer setters, together with the
exact original pedestrian and entity model-index predicates. Dependency owners
observe native operations and simulate existing C zone/timer/pager API boundaries.
Actual production compilation separately verifies the real native game owners.

Coverage includes all 1,536 old/new null/nonnull zone membership and signed/zero/
positive counter combinations in fixed profiles, primary/fallback priority,
missing zones, repeated initialization, front-end reset fields, native null-ped
camera fallback, null and inconsistent pedestrian vehicle state, taxi exclusion,
model-name array borrowing, live UTF-16 pointers and independent local records.
Position copies compare exact bits for signed zero, subnormals, large finite
values, infinities and quiet NaNs. Callbacks change the focused pedestrian after
its predicate, current vehicle after model capture, model game-name storage,
display state during translation, player coordinates between zone queries, prior
membership after the second query, and subordinate update state. Init and Process
ordering and the guarded replay short circuit are independently checked. A
6,000-operation sequence compares all nine operations and retained inactive state.
Native fake taxi/model types have nonzero base adjustments and guards, requiring
the original typed pointer conversions rather than any C layout assumption.

Twenty production source compilations cover the actual adapter, both callers and
C controller across x86/x64 D3D9/GL and Win32 vanilla. Actual x64 D3D9/OpenAL Debug
and Release builds compile and link. The earlier zones audit additionally proves
the entire historical User.cpp producer relocation. Immutable originals, native
layout/type/compiler logs and result records live in build/user-display-c-tests.
No graphical test or game executable was launched.

Run utils/tests/test_user_display.ps1 with optional -Sanitized, -Native,
-Consumers, -BuildProject and -AuditPrevious switches.
