# TxdStore C conversion (applied)

The production texture dictionary store is C11. Its complete tested candidate
remains under `.diagnostics/txd-store-c-tests/overlay`, with an immutable source
baseline under `before`. The baseline was captured after the complete applied
C MemoryHeap migration. Do not regenerate source or fixture files while any
compiler is using them. All checks stay outside the user's `build` directory.

The owner contains all 20 out-of-line operations and the original inline slot
lookup in C11. The two `LoadTxd` overloads become `TxdStore_LoadStream` and
`TxdStore_LoadFile`. Flat `TxdDef` records retain field order and representation,
with an opaque SDK dictionary pointer. Twelve native leaves bind actual SDK
and streaming operations; pool storage, reference policy, saved current
dictionary, retry loops and slot mutation remain in C. The C owner directly uses
the actual `Pool.h`, `General` API and CITAlloc macros.

The original unused `isTxdLoaded` declaration has no definition anywhere in the
source tree. Its corresponding C declaration remains undefined. This conversion
does not invent behavior for an operation the original did not implement.

## Verified owner behavior

- 32 GCC/Clang strict producer and standalone C/C++ API compilations pass on
  x86/x64 under default, MASTER and custom allocator preprocessing
- Sixteen actual MSVC SDK compilations pass for the original C++ producer, the
  new C producer, its C forwarding unit and its complete native leaf unit on x86/x64, with default and
  custom allocator preprocessing
- Eight original/C native runtime comparisons pass: x86/x64, `/Od` and `/O2`,
  default and MASTER. Each executes 36,052 public commands, 2,662 checks, 23
  snapshots, 27 forwarding calls, 528 dictionary creations, two streaming callbacks, five file-open
  attempts, two stream closures and 1,135 external events
- Runtime hashes are `55113da228a6e98b` on x86 and `b938d732aafc1bc9` on x64.
  Both sides also produce the same two expected failed-chunk messages
- Runtime fixtures compile the entire original/C producer, actual `Pool.h` and
  entire `General.c`, with controlled external SDK and allocation boundaries.
  They exercise every defined owner operation, mutable names, case-insensitive
  lookup, duplicate names, saved-current overwrites, all stream stages, chunk
  failure retaining prior slot contents, null read results, file-open retries,
  closure on failed reads, negative reference transitions, callback reentry,
  512 slot reuse iterations, and the exact zero-reference shutdown criterion
- Actual SDK-backed C++ declarations and standalone C consumers agree on every
  record field offset, size and alignment: `TxdDef` is 28 bytes/alignment 4 on
  x86 and 32 bytes/alignment 8 on x64

Native runtime executables run with error dialogs disabled and Windows-only DLL
lookup paths. The game is never launched. Native layout probes include the real
SDK declarations directly, avoiding unrelated rouziclib static initialization
from `common.h`.

## Caller and project integration

All existing callers use the C API. The stream-loading call in Streaming uses
`TxdStore_LoadStream`; filename callers use `TxdStore_LoadFile`. Native SDK
consumers explicitly convert borrowed dictionary pointers back to the original
SDK pointer type. These conversions preserve the record layout and pointer
identity proved by the layout comparisons.

`TxdStore_bindings.c` retains all 27 existing C forwarding ABIs formerly defined
in six renderer native units. Their exact definitions move to C with direct
calls into the C owner. `TxdStore_bindings.h` exposes standalone C declarations.
Both old C++ definitions and all `CTxdStore` references are absent from the
applied source tree. Twenty-nine existing source paths changed, including the
producer rename, and four files were added.

- Sixty whole consumer/producer compilations pass on x86/x64 against actual SDK
  headers, before and after application. Frontend_PS2 remains inactive under
  the D3D9 profile; this is not a PS2 platform build claim
- Both C units select C11 in all 48 evaluated project configurations. All 28
  existing paired C/C++ object names remain distinct. Normal configurations
  contain 423 units; legacy Win32 RW34 configurations contain 422
- Full D3D9 Debug and Release builds link successfully (14,335,488 and 4,954,624
  bytes). The audit inspects actual compiler tracking records for C11 and the
  object timestamps used by both executables
- 1,189 whole historical source projections pass before and after application.
  Complete preceding heap, memory-manager, collision and animation audits pass
- The authoritative gate is `txd_store_stage_audit.py --production --integration`,
  which compares every applied candidate byte and requires all owner, ABI,
  runtime, layout, consumer, project, link, history and predecessor evidence

The successful custom-preprocessing owner compilations do not imply that the
whole game supports the opt-in custom allocator. Its existing heap SDK and
x64 descriptor restrictions remain recorded by the completed MemoryHeap stage.
The project-wide C conversion continues with the remaining C++ owners.
