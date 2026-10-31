# MemoryHeap C conversion (applied)

The production heap is now C11. The complete tested candidate remains in
`.diagnostics/memory-heap-c-tests/overlay`, with the immutable original in
`.diagnostics/memory-heap-c-tests/before`. Do not regenerate either tree while a
compiler is using it. These checks do not use the game's `build` directory or
launch the game.

`src/rw/MemoryHeap.c` contains all 33 original operations as C functions with flat
records. `MemoryHeap_native.cpp` retains 17 primitives for external game and
streaming dependencies. `MemoryHeap_bindings.c` retains 11 existing forwarding
ABIs in C, removing their former C++ implementations from MemoryMgr, ColModel
and RpAnimBlend native units. Game, main and Streaming callers use the explicit
C API. Eight existing paths changed, including the producer rename; three new
files were added. The global heap uses equivalent C zero initialization rather
than a C++ static constructor. Explicit automatic-record initialization retains
the original constructor's partial initialization behavior.

## Verification

- `memory_heap_strict_c.py`: 28 successful GCC/Clang C11 producer and standalone
  C/C++ API compilations; four expected custom-x64 descriptor-size failures
- `memory_heap_fixture.py` followed by `memory_heap_native_runtime.py`: complete
  original/C comparisons using actual MSVC x86, `/Od` and `/O2`, with bug-fixed
  and classic realloc behavior. All four comparisons match: 23,742 commands,
  113,613 checks, 47 state snapshots, eight backing allocations, and hash
  `5b829d3343627211`
- Runtime scenarios include fixed buckets, 1,024 fragmentation/reallocation
  steps, registered-pointer ring wrap and free-index reuse, all three relocation
  entry points, all 11 retained forwarding ABIs, recursive allocation recovery
  and diagnostic dump bytes
- `memory_heap_layout.py`: all field offsets, sizes and alignments match between
  the original C++ declarations and both C and C++ candidate consumers on x86
  and x64. Heap sizes are 396 and 560 bytes respectively; descriptors are 16
  and 24 bytes

- Native consumer compilation passes for all nine affected producer/consumer
  units on x86 and x64, against the real SDK headers, before and after application
- Eight real-SDK producer profile compilations pass; four expected failures retain
  the diagnosed custom allocator SDK and width limitations
- Both new C units select C11 in all 48 evaluated configurations; all 28 existing
  C/C++ object-name pairs remain distinct. Normal projects contain 421 source
  units; legacy Win32 RW34 configurations contain 420
- Full D3D9 Debug and Release builds link successfully (14,337,024 and 4,956,160
  bytes). Actual compiler tracking records prove that the two C units compiled
  with `/TC /std:c11`, including across subsequent incremental links
- 320 whole historical source projections pass before and after application.
  The preceding memory manager, collision model and animation audits also pass
- `memory_heap_stage_audit.py --production --integration` checks every production
  candidate byte, compiler evidence, layouts, runtime comparisons, project
  configuration, full links, history and preceding-owner audit results

The runtime fixture uses complete original and candidate algorithms with
controlled external game owners. It does not prove that the real game supports
the custom allocator. Original custom-x86 compilation already fails on legacy
GTA3 collision APIs and level names absent from this Vice City SDK. Original
custom-x64 additionally fails its explicit 16-byte descriptor requirement. These
limitations remain in place; fixture symbols do not supply production mappings.

During fixture diagnosis, two original behaviors required more precise
observations: coalescing can unlink a fixed-bucket node without decrementing its
counter, and uninitialized allocation slack can retain free-list addresses.
The fixture therefore compares counters and queue lengths independently and
hashes initialized caller payloads separately from block metadata. Reallocation
preservation checks use the previous caller-requested length, rather than
uninitialized physical capacity. A dispatcher argument error for direct block
relocation was also corrected before successful comparisons.

Native runtime executables run quietly with Windows-only DLL lookup paths.
This owner conversion is complete. The rest of the project still contains C++
owners, which remain part of the ongoing project-wide conversion.
