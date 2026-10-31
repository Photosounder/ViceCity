The full text owner is implemented in TextSystem.c. CText, CKeyArray, CData and
CMissionTextOffsets are plain C records. All sixteen complete member operations,
four free Unicode helpers, four explicit initializers and the explicit reverse
ownership cleanup are public C11 operations. TheText, WideErrorString and both
mutable uppercase tables have canonical C storage. All exact TheText method
callers use the explicit receiver APIs.

Text.cpp retains five native boundaries: the live language preference, the two
conditional locale flags, and the existing allocation/release provider. The
native common header redirects allocation to CIT Alloc; routing through that
provider preserves heap ownership rather than silently switching to the CRT
heap. FileMgr, Timer and Messages already expose C APIs and are called directly.

The original global constructor executes before main and registers four owned
buffer destructors. C CRT startup registration and atexit now provide that
lifecycle without a C++ bootstrap object. Text_Destroy releases mission data,
mission keys, main data and main keys in the original reverse declaration order.
Text_Unload separately retains the original message clear and forward release
order, loaded flag and name reset. Initializers write only the original named
fields. Inactive mission entries and padding are not newly cleared.

TextLanguages.h shares the entire existing conditional language enum with C.
Both native menu headers keep their named LANGUAGE enum and qualified aliases
with the same values; no native menu fields change.

Immutable original sources, exact generated production projections, original
native x86/x64 layout probes and source digests are under build/text-c-tests.
The fixture compiles every complete unmodified original method and free helper,
then compares them with the actual C owner through a standalone C11 caller.
Allocation observations include counts, requested sizes, normalized identities
and release order. Each executable returns normally with four canonical buffers
still owned; separate exit logs prove its actual registered cleanup runs in the
original order, including MSVC whole-program optimization and linker collection.
MSYS2 diagnostic executables link their GCC runtimes statically; native runners
suppress Windows error dialogs and no game or GUI fixture is launched.

Runtime cases cover both key formats, sorted search with a distinct supplied
array, empty and missing searches, both complete loader chunk orders, zero and
unknown chunks, all language filenames, mission fallback and replacement,
prefix matching, stale flags after a failed mission selection, short reads,
requested offset increments, byte-at-a-time readers, repeated unload and local
destruction. Every UTF-16 value exercises all six uppercase encoding branches
and the three ASCII converters, including ordered short-circuit locale reads,
mutable uppercase tables, static buffer identity, truncation and retained tails.

Original behavior is preserved, including the byte-reader's diagonal mission
name writes, prefix mission-name matching and ignored short-read counts. Missing
key diagnostics copy an unspecified stack tail beyond their first terminator;
comparisons observe only their initialized visible string. The legacy save/load
and memory-card converters overrun their 256-byte buffers on a 256-character
input; their runtime cases use at most 255 characters. Local inactive mission
entries are seeded after construction before byte observations, since the
original constructor leaves them unspecified. These undefined or unspecified
tails are not claimed as deterministic behavior and no fixes are mixed into the
conversion.

Maintained checks: text_fixture.py audits and prepares complete runtime models;
build/test_text.py runs Clang/GCC comparisons and sanitizer profiles;
build/test_text_native.py runs optimized native x86/x64 comparisons;
build/check_text_types.py checks original layouts, member-to-C signatures and
language values; build/check_text_consumers.py compiles all actual consumers;
build/build_text.py checks both D3D9 builds; build/check_text_audits.py retains
all preceding whole-source audits. The finalizer verifies the complete artifact
matrix and immutable source scope.

The original clock/zone/credits migration entry points now forward their native
adapters through the exact text-call conversion. Historical Text.cpp producers
in the file-manager, timer and message stages additionally prove the complete
relocated C kernel and its native bindings. Exact dependency projections retain
the byte guards for the changed Script.h and menu/text headers. The first broad
audit log is retained, with eleven directly rerun audit blocks recorded
separately before updating the combined 147-stage result. Four additional
original/current runtime comparisons cover the updated credits, zones, user
display and radio-name fixture dependencies; their mock text providers now enter
through the C boundary while keeping the original observations and fingerprints.
