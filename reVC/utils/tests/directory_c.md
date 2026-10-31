`CDirectory` is now a C record. Its complete implementation moved from
Directory.cpp to Directory.c, with seven C functions replacing the constructor,
destructor, methods, overload and reference parameters. Three construction sites,
six destruction sites and nine method calls are converted. Owner allocation and
release remain at their original caller sites. No C++ bridge is needed.

DirectoryInfo retains its 32-byte disk layout: offset at byte 0, size at byte 4
and the 24-byte name at byte 8. CDirectory retains its pointer followed by two
32-bit integers: 12 bytes on Win32 and 16 bytes on Win64. Entry allocations and
releases continue through the actual CITAlloc header, preserving allocator
ownership and diagnostic metadata. Initialization writes capacity before count,
matching the original class member initialization order. Shutdown releases only
the entry array, without clearing fields or releasing the containing record.

The complete original implementation is compared against the C11 implementation
and a separate C11 caller. The fixtures use actual General and FileMgr C code,
real binary files in an isolated build sandbox, and guarded entry allocations
at the CITAlloc boundary. They cover full record bytes, duplicate handling,
first-match case-insensitive lookup, unchanged outputs on failure, pointer
aliasing, input preservation while encoding image bits, capacity assertion order,
zero-size and failed allocations, partial reads after a complete initialized
record, file round trips and repeated ownership cycles. The capacity assertion
remains before the FIX_BUGS duplicate check. Every positive read is still added,
including a partial read. Undefined original inputs such as a partial first
record, invalid image shifts and missing-file reads through handle zero are
excluded; this conversion does not introduce new behavior for them.

Twelve Clang/GCC O0/O2 original/C comparisons pass across default, no-FIX_BUGS
and MASTER profiles. Six Win32/Win64 O2/GL/LTCG/OPT:REF comparisons use actual
project headers for the original implementation and record consumers. Each run
performs 68,626 to 68,653 checks, with 261 or 262 guarded allocations and matching
releases. Three Clang O1 comparisons trap undefined behavior and float-cast
overflow. Twenty-three native production compilation checks cover all four
implementation consumers and the C unit, including feature profiles. All 92
preceding whole-source migration audits pass.

Each original/C pair has the same fingerprint. GCC and Clang fingerprints differ
because the fixture's original C++ argument evaluation order consumes random
inputs differently; comparison is within the same compiler and profile. No
cross-compiler identity is asserted. The real CITAlloc boundary is intercepted
by the fixture to verify allocation ownership; the allocator's internal heap
implementation is not exercised.

Ten complete source/project transformations are audited against immutable
originals in build/directory-c-tests/before. Directory.cpp is removed, and Visual
Studio explicitly compiles Directory.c as C11. Touched files use LF endings.
Run utils/tests/test_directory.ps1 with optional -Sanitized, -Native and
-AuditPrevious switches. Native fixture-only abort definitions satisfy unused
rouziclib header-table dependencies and fail if called. Unused SDL2/GLEW auto-link
requests are suppressed only in fixture linking. External CITAlloc headers are
system includes for GCC/Clang; the new C implementation still compiles with
-std=c11 -pedantic-errors.

No graphical tests were run. Full game linking and gameplay remain unverified.
