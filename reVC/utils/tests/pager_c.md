The pager is now a plain CPager record containing the original eight PagerMessage
records, seven complete C11 operations with explicit queue pointers, and one
canonical C-owned Pager_state. The CPager methods and CUserDisplay::Pager static
member are removed. All five consumers use the C instance/API. The queue continues
to borrow the original writable UTF-16 source pointers, including interior aliases.

PagerQueue.c contains each complete original algorithm. Pager.cpp contains five
typed native boundaries: four lazy camera/HUD/message-channel reads and the
synchronous HUD message copy. The pager uses the C message API directly for
UTF-16 length, number expansion and previous-brief insertion, and uses the C
Timer API directly. All queue decisions,
priority scans, record shifts, local buffers, scroll slicing and field writes are
in C. The unique C source basename avoids object collisions. Visual Studio
registers the source as C11, and existing CMake/Premake globs include it.

Native probes before editing established message/queue sizes of 44/356 bytes on
x86 and 48/392 bytes on x64. New probes compare every original field offset/type,
alignment, the complete eight-record array and all seven member/API signatures.
The original records were already trivial: construction, copying and destruction
remain trivial. The canonical instance retains its original static zero state.

Twenty portable comparisons cover Clang/GCC O0/O2 with default, classic language,
MASTER, unfixed and GL profiles. Nine MSVC O2/LTCG comparisons cover default, GL,
MASTER and classic on both architectures plus Win32 vanilla. Five Clang O1
comparisons trap undefined behavior and float-cast overflow. Every original/C
pair matches. Each run observes 10,113 complete states, 5,124 length calls, 2,724
expansions, 2,092 brief insertions, 3,888 HUD copies and eight stateful callbacks.
Each run performs 654 independent assertions on x86 and 690 on x64; the difference
comes from checking every byte of the original zero-initialized queue. The
fingerprint includes original padding and therefore differs by layout:

    x64/default: 93f08d131370ea76
    x64/classic: 8cf3c5d7644ec8db
    x86/default: 00e42d14964d1842
    x86/classic: 41bab2faae635377

Win32 vanilla disables MORE_LANGUAGES and matches the x86/classic result. All
compilers use explicitly sequenced random arguments in the test driver; input
traces no longer depend on C++ argument evaluation order. Native execution uses
the matching freshly compiled sequenced executable names.

The oracle compiles all seven untouched original method bodies and original
record declarations, exposing private fields only for observation. Both paths
enter through separate C11 callers; the original C caller sees only an opaque
queue pointer. The converted path compiles the actual production C unit and all
eight complete native boundaries. Native tests use real project common/types,
config and Timer headers. Fixtures compile the entire original length, number
substitution, previous-brief insertion, HUD-copy and ASCII-to-UTF-16 helpers.
Observation wrappers retain those complete native operations while recording
payloads and changing state at selected call boundaries. Actual production
adapter/caller compilations separately verify all real game-owner bindings.

Checks cover static zero state; partial Init/Clear retaining inactive fields,
padding and display width; stable equal-priority ordering; full queue rejection
with the original eager string work; insertion into holes; complete record
shifts; native history deduplication; signed unused fields; all six numbers;
borrowed and interior text pointers; null numbered messages; UTF-16 and Japanese
placeholders; negative scroll padding and widths; source terminators; strict
time comparisons and unsigned wrap; all camera/HUD/two-channel restart gates;
display-before-update ordering; first-hole expiry behavior; and local receivers.
Callbacks change width/time/text after length capture, change scroll fields after
expansion, change registration/timing after HUD publication, and replace a newly
promoted head before display. A 10,000-operation sequence compares complete queue,
inactive-record, history and HUD state. Pointer identities are normalized without
hashing process addresses. Tests respect the original fixed-size numeric/string
temporary bounds; they do not claim to repair existing message-format overflows.

Thirty production compilations cover all updated C++ units and the C queue on
x86/x64 D3D9/GL and Win32 vanilla. Actual x64 D3D9/OpenAL Debug and Release projects
compile and link. The earlier Timer source audit additionally proves the entire
old pager producer relocation. Immutable originals, compiler/build logs and result
records live in build/pager-c-tests. No game or graphical test was launched.

Run utils/tests/test_pager.ps1 with optional -Sanitized, -Native, -Consumers,
-BuildProject and -AuditPrevious switches.
