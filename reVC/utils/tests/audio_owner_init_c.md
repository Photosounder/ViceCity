# C owner record and initialization

AudioManager.h now exposes a C-compatible typedef struct containing the owner's
original stored fields. Fixed-width C scalar names replace equivalent C++ aliases.
Remaining methods and constructors are visible only to C++; game-specific headers
and validations are similarly guarded. This is an intermediate conversion: the
owner's remaining method implementations and lifetime callback still use C++.
There is no inheritance, duplicate record, changed field order, or copied runtime
owner state. The existing AudioManager global now has C linkage.

AudioManagerInit.c is an actual C11 module implementing constructor state setup
and the five-entry random-table fill. The original C++ constructor calls its C
initializer at the same registered startup point. Service calls the C random fill
at its original first statement. The former random-table member is removed.
Constructor defaults, conditional writes, and partial initialization remain exact;
there is no blanket reset of fields the original constructor left untouched.
The destructor and conditional Terminate call remain unchanged.

Random.h declares the existing myrand/mysrand functions with C linkage, and
common.h includes it. Their implementations and seed state in re3.cpp are
unchanged; native objects confirm the real definitions match C callers. Owner
construction still consumes exactly five ordered RNG calls before main. No RNG
initialization is moved to DMAudio_Initialise or later game startup.

The owner header loads config.h before channel enums. This is necessary for C
consumers to select the same configured channel capacity as game C++ consumers.
Native compiler-emitted ABI constants compare the original C++ record, current
C++ record, and actual C record: all 54 fields match in offset, size, and alignment.
x86 owner size remains 26200/alignment 4; x64 remains 28376/alignment 8.

Run utils/tests/test_audio_owner_init.ps1. The generator audits complete state
setup, random fill, record fields, owner edits, header edits, and RNG declaration
change against saved sources, and confirms re3.cpp remains byte-for-byte unchanged.
The lifecycle fixture uses actual constructor/destructor bodies, original stored
field declarations, the actual game RNG arithmetic, and the real C initialization
modules. Quiet probes observe construction order, five startup draws, later table
updates, partial initialization on live prefilled records, termination gates,
script-manager reset, and reverse destruction order. A C caller reads the C-linkage
global through the actual owner header. The shutdown game operation is mocked;
this does not run the game or its audio device.

Partial-state tests prefill an already-live record, rather than relying on bytes
written before placement construction to define untouched C++ members. Destructor
state is observed while the record remains alive. These choices avoid testing
indeterminate values or compiler lifetime optimizations as if they were required
legacy behavior. The actual global still receives normal static zero-initialization.

All twenty Clang/GCC O0/O2 comparisons pass in default, vanilla,
external-3D-disabled, reflections-disabled, and PS2-shaped configurations. Native
MSVC x86/x64 comparisons pass with digests 5163739539343347759 and
10530692831227387711. All 290 consumer compile checks pass. Native MSVC also
compiles the actual game owner/integration files and actual re3.cpp RNG definitions.
Object symbols retain the original owner initializer/destructor callbacks and show
matching C global, initializer, random-fill, and RNG linkage.

The existing Visual Studio project/filter registers the C11 module and Random.h;
CMake/premake already collect C sources. Sources saved before conversion, native
runtime/ABI scripts, logs, and verification.json are in build/audio-owner-init-c-tests.
Prior relevant source audits account for this initializer/header migration while
continuing to verify their converted operations. No full linked game build or
in-game playback has been performed.
