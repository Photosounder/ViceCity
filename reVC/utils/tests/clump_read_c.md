# ClumpRead C conversion (applied)

Production now uses the complete C clump reader. The tested candidate remains under
`.diagnostics/clump-read-c-tests/overlay`, with the immutable baseline under
`before`. The baseline was captured after the applied C TexRead conversion.
Do not regenerate source or fixtures while compilers are using them. Checks
stay outside the user's `build` folder, and the game is never launched.

All seven original operations are C11: first- and second-stage
geometry-list reads, geometry-list deinitialization, atomic loading, clump start,
clump finish and cancellation. Their complete original bodies retain the shared
geometry counter, saved stream position, serialized clump header, allocation
behavior, assertions, reference-release order and early returns. The atomic
reader's original reset of `numberGeometrys` is preserved.

The application-owned geometry-list, atomic-binary and clump-header records are
flat C records. SDK objects cross the C API as opaque pointers. Twenty-three
native primitives bind actual allocation, stream, geometry, atomic, clump and
frame operations or live SDK metadata. No complete loading algorithm moves back
into a native C++ forwarding function.

## SDK frame-list storage

The actual SDK frame-list object remains one zero-initialized native singleton,
as in the original owner. C borrows its opaque handle; count and element access
use native metadata primitives. Reads and deinitialization operate directly on
that same SDK object, and cancellation resets its count after deinitialization.

This is deliberate: `rwFrameList` aliases the SDK's `rw::FrameList_`, whose
`streamRead` writes count and pointer fields before invoking frame plugin
callbacks. The implementation in `vendor/librw/src/frame.cpp` uses the same
object throughout. Copying fields through a temporary native record would delay
visible mutations and could overwrite state changed by reentrant callbacks.
Reinterpreting a separately declared C record as the SDK class would also create
incompatible-record aliasing. Opaque storage retains the SDK's actual type,
stable address, lifetime and live mutation behavior without either problem.

The SDK frame pointer array likewise remains SDK-typed. C requests individual
borrowed elements through a metadata primitive, avoiding reads of an SDK
`RwFrame*` array through an incompatible `void*` element type.

## Verification

- 24 GCC/Clang strict C11 producer and standalone C/C++ API compilations pass on
  x86/x64. Producer profiles include legacy declarations, LIBRW, MASTER and
  custom allocator preprocessing
- Twelve MSVC compilations pass against the actual SDK headers for the whole
  original C++ producer, the C candidate and the native primitive unit on
  x86/x64, with default and custom allocator preprocessing
- Actual SDK-backed C++ declarations and standalone C consumers agree on size,
  alignment and every field offset for all three migrated records. Geometry
  lists are 8 bytes on x86 and 16 on x64; atomic binaries are 16 bytes and clump
  headers 12 bytes on both. The SDK frame list retains its original SDK type
- `clump_read_stage_audit.py` verifies whole source identity, all seven complete
  operations, actual SDK object access, record layouts, compilation evidence and
  exact production integration

The proprietary SDK directory remains absent. Actual SDK compilation evidence
uses LIBRW D3D9; legacy C syntax passes but proprietary SDK linking is unverified.
Custom-preprocessing owner compilation does not imply the whole game's existing
custom allocator restrictions are resolved.

## Runtime and integration evidence

- Twelve complete original/C comparisons pass under actual MSVC x86/x64,
  `/Od` and `/O2`, LIBRW, legacy and MASTER preprocessing
- Every comparison executes 200 public commands and 2,521 checks, creates 123
  geometries, 49 atomics, 20 clumps and 126 controlled SDK frames, exercises two
  allocation failures, and releases all 96 actually allocated raw arrays
- Native event counts are 1,818 for LIBRW/MASTER and 1,745 for legacy. Hashes are
  `4a0f1a9152122009` / `bb3f6f0c0b9d8080` on x86 and
  `93e67ab742e6fd71` / `2926c83e5722a680` on x64 (LIBRW or MASTER / legacy).
  Corresponding original/C results match at both optimization levels
- Cases cover odd/even/empty geometry counts, structure and geometry chunk/read
  failures, allocation failure, inline/shared geometry, no-frame atomic reads,
  12/16-byte atomic headers, exact flag bits, SDK reference retention/release,
  the atomic reader's shared-counter reset, both clump header versions, saved
  positions on an independent resumed stream, first-stage cancellation, every
  second-stage failure path and clump-creation failure
- The SDK frame-list callback cancels the current transaction during the read.
  Both implementations immediately expose zero count and a null array in the
  very SDK object currently being read. This specifically detects delayed-copy
  frame-list adapters that overwrite or hide live callback mutations
- SDK mock frame-object retention after original array-only cleanup is observed
  explicitly: 72 frames and zero geometries remain in the controlled SDK model.
  The comparison preserves the baseline rather than silently adding destruction
- Eight whole changed producer/consumer units compile against the real SDK on
  x86/x64 before and after application
- All 48 evaluated configurations select the owner as C11; all 28 existing
  paired C/C++ object names remain distinct. Normal configurations have 426
  units; legacy Win32 RW34 configurations have 425
- Full D3D9 Debug and Release builds link successfully. The audit requires actual
  `/TC /std:c11` compiler records and linked object timestamps
- 172 whole historical source projections pass before and after application.
  Complete preceding texture, dictionary-store, heap, allocator, collision and
  animation audits remain required

Four existing source paths change: the producer rename, FileLoader and Streaming
clients, and RwHelper declarations. Three files are added: the self-contained C
API, native boundary declarations and native SDK storage/primitive unit. Clients
call the C entry points directly; the returned opaque clump pointer is converted
to the caller's original SDK type. No C-to-C++-to-C algorithm wrapper is retained.

The first-stage allocation failure's positive count/null array state is preserved.
The runtime controller observes that state and begins a valid empty transaction
before any further cancellation, avoiding an invalid original follow-up call.
Partial read failures are observed before explicit caller cancellation. Fixture
cleanup does not replace producer cleanup or modify original function bodies.

The authoritative gate is
`clump_read_stage_audit.py --production --integration`, including every applied
candidate byte, actual SDK and strict compiler evidence, full runtime variants,
layouts, project/link evidence, historical proofs and preceding-owner audits.
The overall project conversion continues with remaining C++ owners.
