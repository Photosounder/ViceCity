VariableConsole.c owns the canonical variable-console state, all 91 entry
records, all 26 former methods and an explicit C construction operation.
CVarConsole and its fourteen Add overloads are gone. VarConsoleState and
VarConsoleEntry are plain C records; each typed Add operation has a distinct
C name and every operation takes an explicit owner pointer. Eleven production
caller files use the C API, including the actual uint8 clock registration and
the bool8 coordinate-update flag. Their types remain distinct from native bool.

The conditional pre-main constructor is replaced by matching static C state:
page one under FIX_BUGS, otherwise the original all-zero global. Actual callers
are in runtime initialization/input functions and do not register entries from
other global constructors. Explicit construction retains the original conditional
Initialise call. Initialise writes exactly the original five fields; it leaves
the page count and every entry record untouched. Entry sizes remain 112/160
bytes and controller sizes 10,216/14,584 bytes on Win32/Win64 respectively.

All duplicate checks still compare text pointers, rather than string contents.
Typed registration keeps the original pointer fields, signed step/bounds
storage, assignment order and stale fields. Removal keeps ascending whole-record
shifts and leaves the final stale slot intact. Numeric and boolean editing keep
the original narrowing, unsigned wrap, signed-view lower-bound comparisons,
clamp/wrap branches and function callbacks. C entry pointers replace the two
C++ references while retaining the same live array-element identities.

Pagination retains count/30+1, including its extra page at exact multiples of
thirty. Display retains every format string, the conditional 64-bit formatting,
actual UTF-16 buffers, live selection/background checks and live loop bounds.
Input retains fresh second-pad queries, short-circuit order, page changes and
callback effects. Process still runs input before sorting pages and displaying.
Open still returns the original raw uint8 flag. No capacity clamp, null-pointer
guard, duplicate-name comparison or navigation repair was added.

Thirty-one typed native boundaries retain font operations, actual screen-scaling
macros, ASCII conversion and individual pad queries. Color arguments are copied
into real native CRGBA values, and UTF-16 pointers use the actual native scalar
type. The C kernel contains all controller decisions and formatting. Native
common/config, font, pad, screen and drawing declarations remain unchanged.

The fixture audits fifteen complete source/project changes, both new production
files, all fourteen overload replacements and the complete records against
immutable originals. The original program compiles every complete old method
and executes the real conditional global constructor. A constructor probe runs
the original constructor on a fresh object and observes its five initialized
fields without reading uninitialized automatic fields. Both programs expose
only named canonical fields for observation and mutation, excluding padding.
Both link actual Draw.c; the original screen macros are retained in portable
tests and real native common headers are used in native tests. A separate C11
caller enters all 27 operations.

The earlier particle and surface audits now compare the entire old main.cpp
after the explicit console-call rewrite. Their original byte guards still apply
to every other native dependency, and every unrelated statement in main.cpp is
covered by the complete transformed-file comparison. Initial failures and both
successful targeted rechecks are retained with the combined 131-audit log.

The corpus checks every named field in all 91 slots, all fourteen value kinds,
pointer-identity duplicates, equal text at different addresses, all bounds
policies, negative/out-of-range values without signed 32/64-bit overflow,
complete capacity, first/middle/last removal, empty and boundary page counts,
individual input queries, short-circuit combinations and the close chord.
Callbacks replace their active entry and change subsequent live input. Font
callbacks change selection, bounds, later scalar values and screen state while
display is in progress. Initialisation and construction preserve untouched
records/page counts; checks also cover unknown type codes and raw open bytes.
Each run observes 6,985 states, 6,009 registrations, 842 edit operations, 61
function callbacks and 1,061 individual pad queries.

Twenty Clang/GCC O0/O2 comparisons cover default, plain, MASTER, unfixed and GL
profiles. Five Clang comparisons enable undefined-behavior, float-conversion
and indirect-function-type traps. Seven optimized MSVC comparisons cover Win32
default/GL/MASTER/vanilla and Win64 default/GL/MASTER. Native probes verify every
field offset/type, both whole-record layouts, all enum values, all 27 C contracts,
all 31 native boundary contracts and standalone C11 public/backend headers.

Sixty-five production compilations cover all eleven real callers, the native
boundary and the C unit across five architecture/feature profiles. Actual Debug
and Release x64 D3D9/OpenAL projects compile and link. All fifty configurations
retain unique objects for all twenty-eight paired C/C++ units. All 131 preceding
whole-source audits pass. No game or GUI test is launched.

Run utils/tests/test_var_console.ps1. Optional switches select Sanitized, Native,
Consumers, BuildProject and AuditPrevious. Immutable originals, generated inputs,
logs and manifests are in build/var-console-c-tests.
