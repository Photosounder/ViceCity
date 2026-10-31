# Complete memory manager conversion

MemoryMgr.cpp is now MemoryMgr.c. All seven complete original operations own
initialization, malloc/realloc/calloc/free selection, high-water state and aligned
allocation headers in C. MemoryMgr.h is a self-contained C API with C++ linkage
guards. Nine existing source/header paths changed, with complete original and
candidate trees under .diagnostics/memory-mgr-c-tests/before and overlay.

The normal allocator was already redirected by the actual cita_windows.h macros
from rl.h. The C producer imports that same external header instead of replacing
normal allocations with CRT malloc. Original function metadata is preserved;
diagnostic filenames and lines naturally identify the new C source. Existing
native callers include MemoryMgr_native.h, which keeps the complete original SDK
table declaration and tracking macros beside the new C API. MemoryMgr_native.cpp
retains the exact original RwMemoryFunctions initializer as native SDK data and
only four custom-heap primitives. No memory-manager algorithm remains native.

Preserved policy:

- Init selects the original 128 MiB custom-heap request and existing PS2 error
  branch, without resetting pMemoryTop. The normal branch remains empty.
- Normal malloc, realloc, calloc and free use actual CITAlloc macro declarations.
  Custom requests still narrow to the heap's original uint32_t method parameters.
- High-water comparisons read live state after allocator calls. Calloc still
  updates by one element's size, not count*size. Custom calloc clearing and
  custom null-free suppression remain conditional on FIX_BUGS.
- Both original aligned-allocation formulas and the hidden original pointer
  header remain intact. FIX_BUGS_64 still selects the pointer-width-safe branch
  even with classic behavior on x64. Classic x86 retains its distinct formula.
- Assertions keep the original handler and MASTER expression-evaluation policy.

Validation:

- Twenty Clang/GCC x86/x64 strict C11 producer and independent standalone C/C++
  API compilations pass, including MASTER and the custom allocator. CITAlloc is
  a system include for those diagnostics: its preexisting non-prototype C
  declaration is not rewritten or mistaken for project C11 code.
- Eighteen complete native D3D9 source compilations pass before application and
  eighteen from production. GLFW and POSIX source guards remain inactive in
  this Windows/D3D9 compilation; their include changes preserve the literal
  original native compatibility contract. Seven real normal/custom producer
  and primitive compilations pass in each phase.
- The unchanged custom allocator's x64 HeapBlockDesc size assertion rejects both
  the complete original producer and new native primitive unit. These two
  expected failures are recorded separately, never counted as successful builds.
  Owner policy is independently compared under actual MSVC x64 using a primitive
  heap fixture; this does not claim the existing native heap supports x64.
- Sixteen whole-producer Clang/GCC O0/O2 comparisons, four undefined-behavior
  sanitizer comparisons and eight actual MSVC x86/x64 comparisons pass. Both
  entire producers and actual SDK callback-table initializers participate.
- Every run has 2048 scenarios, 8288 balanced allocations, 4096 SDK table calls
  and 96 reentrant allocator calls. Inputs cover reallocation preservation,
  calloc bytes and count handling, null frees, high-water state, alignment and
  recovery of the original allocation. Custom heap initialization occurs twice
  at the exact original size. Tracking macros check the original public function
  names. No CITAlloc implementation or custom heap algorithm is duplicated.
- Diagnostic backing storage is a real untyped aligned allocation. Pointer
  comparisons use one live allocation, and typed alignment headers do not alias
  a declared char-array object. Final high-water observation is evaluated before
  printf to avoid compiler-dependent diagnostic argument evaluation order.
- Check counts are normal 525185, custom fixed 500325 and custom classic 502373.
  x64 high-water offset is 5115. x86 fixed offset is 5111, classic is 5108.
- x64 hashes: normal/fixed and normal/classic db11797266254738, custom fixed
  a06a8c6ea9e3a394, custom classic c607dbdd39da0055. x86 hashes respectively
  8e0302ee8ef6ddca, c2733943b624f35b, 91b531c64187312a, 19cd48cc7acd2429.
- GCC diagnostics use static runtimes, audit imports for unwanted GCC/MSYS DLLs,
  and execute on a Windows-only runtime PATH. Native error dialogs are disabled.
- All 48 evaluated configurations select MemoryMgr.c as C11 without object
  collisions. Source count is 419, or 418 for legacy Win32 RW34; paired C/C++
  basenames remain 28. All 351 complete historical source/creation projections
  pass before application and against production. Snapshots predating the
  animation SDK leaf unit require its exact complete creator projection.
- Full isolated D3D9 Debug and Release builds/link pass. Executable sizes are
  14334464 and 4956160 bytes. Neither game executable was launched. Every actual
  production source byte matches the coherent candidate used by both builds.

The col_model migration endpoint now wraps memory_mgr; stats_successor records
the explicit MemoryMgr.cpp -> MemoryMgr.c rename. apply_memory_mgr.py required
all staged proofs before application. Earlier plugin, clump, sequence,
association, frame-callback and model final audits pass against exact complete
successors. The changed animation leaf remains an exact creator projection with
only the explicitly migrated native memory include. Archived model project
metadata uses its private stage projection instead of later source counts.

memory_mgr_stage_audit.py --production --integration requires all source/runtime
evidence, both full links and actual production consumer compilations. Overall
project conversion remains active. No git or GUI tests were used; build/ remains
untouched. Do not regenerate immutable source or fixture trees while a compiler
using them is still live.
