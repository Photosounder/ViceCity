# Pedestrian types and stats in C11

`src/peds/PedestrianData.c` owns the 23 pedestrian type allocations and 40 pedestrian statistic allocations. It implements all 14 original type operations (including the seven former inline accessors) and all four statistic operations. `PedType.h` exposes plain records, unchanged enum values and C linkage. The ten original caller files use these functions and canonical arrays directly.

`PedType.cpp` contains four small native boundaries for the existing allocation/free provider, debug formatting and unknown-type diagnostic. It contains no pedestrian data parsing, lookup, initialization, mask mutation or serialization decisions. The kernel has a unique basename, preserving the earlier fix for C/C++ object-file collisions.

The original `std::allocator<T>().construct(pointer)` value-initialized these trivial classes. The C implementation retains malloc and explicitly clears each record before applying the original assignments. This includes type `unknown3`, the statistic name bytes around the unusual `m_name[8] = 'R'`, and padding. Shutdown retains the original dangling slot values. It introduces no pointer clearing, validation, ownership or parser changes.

The `PTP` save block remains 744 bytes: an eight-byte header followed by 23 complete 32-byte records. Loads write existing allocations. Header checks retain the native `re3_assert` callback and the existing `MASTER` behavior. The parsers retain their original grammar, token handling, normalization and narrowing conversions. Comparison inputs satisfy their existing line/token limits and omit blank/all-space lines and unknown record definitions; parser hardening is a separate change.

Run from the repository root:

```powershell
./utils/tests/test_ped_data.ps1 -Native -Consumers -BuildProject -AuditPrevious
./utils/tests/test_ped_data.ps1 -Sanitized
```

The console fixture compiles the entire immutable original class definitions and implementation, alongside the new real C kernel. It links the actual `General.c` and `SaveBuf.h`. Native runs include the actual project common header and actual four-function C++ bridge; mocked allocator/debug endpoints avoid launching the game. It compares every record byte and observable host operation, with independent expectations for defaults, enum/flag mappings, all accessors, signed threat arguments, first-match stats lookup, numeric parsing, name truncation, short files, pointer identity and aligned/unaligned saves. Corrupted headers exercise all five returning assertion callbacks.

Validation consists of 20 Clang/GCC comparisons (O0/O2, five profiles), seven native MSVC comparisons (x86/x64, D3D9/GL/MASTER plus x86 vanilla), five sanitizer comparisons, 55 actual production compilations, original/native record and function signature checks on both architectures, D3D9 Debug/Release linking, object uniqueness in all 50 configurations and 136 preceding complete-source audits. Earlier parser/caller snapshots still prove the full relocated kernel; dependency guards permit only the exact known pedestrian transformation.

Each ordinary run reports 84 record snapshots, 225 allocations/releases, 36 file loads, 72 directory operations, 30 diagnostics, 24 unknown-type diagnostics, 48 save round trips, 708 lookup checks and 2,415 threat-mask cases. The record/operation hash is `aaa7192b8d7aa72a`. Validation-enabled profiles observe 240 returning header assertions; MASTER/vanilla profiles observe zero.
