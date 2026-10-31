SetPieceControl.c owns all eight complete set-piece operations, the canonical
debug flag and count, and all 96 persistent records. CSetPiece and CSetPieces
are removed. SetPiece and its two-float point fields are plain C aggregates.
Instance operations take an explicit owner pointer. Three production caller
files use the C API, including six explicit by-value point packets read from
the script parameters. A unique source name avoids C/C++ object collisions.

The controller retains initialization, capacity checks, exact Min/Max comparisons,
stale timestamps on insertion and the original eight-frame scheduling. Record
updates retain unsigned cooldown arithmetic, inclusive trigger boundaries,
wanted/player-vehicle gates, ordered dot products, every single/two-object spawn
branch and partial-spawn cleanup. The asymmetric second-car blocking assignment
still writes the first car's cruise speed. This conversion makes no gameplay
repair, capacity policy, null guard or allocation recovery change.

Both spawn helpers retain native pool allocation and construction, vertical
placement, actual collision filters, collision rejection, explicit destruction,
raw pool release and world insertion. The C kernel performs direction subtraction,
the original three-component normalization including its zero fallback, basis
selection and multiply-before-divide movement calculation. Pedestrian objectives,
vehicle autopilot fields and timers are written in the original sequence with
the same live record accesses after native operations.

Forty-two typed native boundaries retain the original owner methods, constructors,
bitfields and canonical field pointers. Value packets copy named components;
the C code assumes no native owner layout. The vertical-line adapter copies
hit height only after a successful native query. Inspection of the unchanged
world-query implementations confirms they consume vector references synchronously.
Native enum constants are checked with static assertions. Status values use the
existing C-compatible production header.

The record remains 56 bytes with four-byte alignment on Win32 and Win64. Every
field offset and point component matches the old class. The save block remains
5,380 bytes. Save/load retain the original aggregate assignment, alignment
preconditions, native padding behavior and complete 96-record transfer. They
use the existing C SaveBuf API and retain its size validation. No initialization,
padding zeroing or count validation was added to loading.

The source fixture checks seven complete changed source/project files and both
new production files against explicit transformations of immutable originals.
Native dependencies retain byte guards. The preceding timer and variable-console
audits verify the complete relocated set-piece algorithms in their earlier
snapshots as well as the current backend. The migration chain forwards all
later caller/project changes while retaining complete-file comparisons.

Runtime fixtures compile every complete original method and the actual C11
kernel. A separate C11 caller enters all eight operations. Named-field adapters
observe canonical records without reading padding. Narrow native vehicle/cop,
world and wanted shells control external behavior; they do not claim to reproduce
the native owner layouts. They use the actual AutoPilot declaration/constructor,
native vector/matrix types, original vector normalization and matrix rotation
algorithms, and the complete actual C pool. Native fixtures use actual production
common/RenderWare headers with a quiet CRT allocator adapter.

The corpus independently varies type, wanted level, player-vehicle presence,
collision outcome, trigger position and movement direction. Explicit assertions
require success and first-spawn rejection for every supported type, plus second-
spawn rejection and cleanup for every two-object type. It checks the asymmetric
cruise-speed write directly. Native callbacks perturb timers and later target
coordinates to check live reads. Direct helper calls check hit/miss placement,
zero-direction handling and collision rejection. Cooldown tests include exact
boundaries and unsigned wrap. Frame tests cover counts 0, 1, 7, 8, 9, 95 and 96.
Insertion tests cover full capacity and initialization of the count alone.

Eight saves and eight loads per comparison check every serialized byte and guard
at four valid alignments, including nonzero record-padding bytes from an existing
save followed by another save. Counts remain valid and native spawn pointers
refer to actual constructed raw-pool objects. Player-vehicle presence uses an
undereferenced non-null sentinel. Coordinate/speed queries are counted without
imposing an order on the original function arguments.

Each default comparison observes 8,267 states, 4,873 native calls, 3,206 record
updates, 56 frame updates, 244 car and 196 cop constructions, 440 destructions,
440 collision queries and 96 occupant calls. Twenty Clang/GCC O0/O2 comparisons
cover default, plain, MASTER, unfixed and GL. Five Clang comparisons enable
undefined-behavior, float-conversion and indirect-function-type traps. Seven
optimized MSVC comparisons cover Win32 default/GL/MASTER/vanilla and Win64
default/GL/MASTER. Default hash is 667469f94d2c0849; MASTER/vanilla omit debug
registration and have hash dde199f2839338aa.

Twenty-five production compilations cover the backend, three caller files and
C11 kernel in five native profiles. Actual native probes check all eight public
operations, every saved-field offset, all 42 boundaries, constructor contracts,
native field types and both complete collision signatures. Actual x64 D3D9 Debug
and Release builds link. Object-name checks cover all 50 configurations and 28
C/C++ pairs. The preceding 134 complete-source audits are also checked.

Run utils/tests/test_set_pieces.ps1 for portable comparisons. Add -Sanitized,
-Native, -Consumers, -BuildProject or -AuditPrevious for the respective checks.
The evidence checker is build/finalize_set_pieces.py. All checks are quiet
compiler/console checks; game rendering and gameplay remain for manual testing.
