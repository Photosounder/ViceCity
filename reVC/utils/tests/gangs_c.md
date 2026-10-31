# Gang data in C11

`src/peds/GangData.c` owns nine gang records and nine attack-policy flags. All sixteen original operations, including the four inline accessors, are C functions. The former `CGangInfo` constructor is replaced by constructor-equivalent static C initializers. Both canonical arrays remain private to their owning C file. The eleven full caller files use the C API and actual canonical records.

`Gangs.cpp` contains only one typed native adapter, `Gangs_BackendHasModelLoaded`, which enters the existing streaming predicate. Both the conditional second streaming lookup and every random/override decision remain in C. The adapter keeps no gang state or gameplay choices.

Before editing, native x86 and x64 probes established that the current original record is **24 bytes**, aligned to four bytes. The old `VALIDATE_SIZE(..., 0x10)` annotation was stale and inactive because `CHECK_STRUCT_SIZES` is disabled. All three model IDs and both weapon fields are currently full-width signed integers. The C representation preserves their actual types and offsets, with a compile-time 24-byte assertion. The `GNG` save block remains **224 bytes**: an eight-byte header and nine complete records.

Global construction originally initialized all model/override fields to minus one, weapon fields to unarmed, static padding to zero and attack flags to false. The C data is ready before any explicit initialization call. `Initialise` retains its partial-reset behavior: eight gangs receive default models/vehicles, the golfer receives two golf clubs, and `FIX_BUGS` resets all nine overrides. Other weapons, the ninth gang's models/vehicle, padding and attack flags retain their prior values. `LoadAllGangData` still initializes before checking the header, then assigns complete saved records; attack flags remain outside the save block.

Both save and load retain aggregate record assignment rather than introducing a different copy policy. Comparison programs check every byte, including seeded nonzero padding, at four valid aligned buffer offsets. They also exercise returning header assertions and confirm initialization happened before each rejected header. The original native assertion callback and MASTER behavior are preserved.

Run from the repository root:

```powershell
./utils/tests/test_gangs.ps1 -Native -Consumers -BuildProject -AuditPrevious
./utils/tests/test_gangs.ps1 -Sanitized
```

The console oracle compiles the complete immutable original constructor, records and class implementation. Both variants link actual `General.c`, `SaveBuf.h` and the existing streaming load-state comparison. Streaming instrumentation observes each query and can mutate the canonical second model between queries, verifying live reads and short-circuit order. Record transfers use byte copies to avoid type-punning between original and converted classes. Independent checks stay enabled in MASTER as well as other profiles. Inputs retain original valid gang indices and streaming-table indices.

Coverage includes static defaults before initialization, all sixteen operations, signed-byte override categories, RNG thresholds under both 16-bit and 15-bit masks, every supported/unknown byte streaming state, live model changes between queries, both inclusive gang-pedestrian boundaries, every valid nongang pedestrian type, repeated initialization, extreme signed model/weapon values, save/load byte identity, stable record addresses, policy persistence and corrupt headers.

Validation comprises 24 Clang/GCC original/C comparisons (O0/O2, six profiles), nine MSVC comparisons (x86/x64, D3D9/GL/MASTER/15-bit RNG plus x86 vanilla), six sanitizer comparisons, 65 actual production compilations, original/native record and function-signature checks on both architectures, both actual D3D9 Debug/Release links, all 50 evaluated configurations with unique objects, and 138 preceding whole-source audits. Earlier general-helper snapshots prove the complete relocated gang implementation as well.

Each comparison reports 83 whole-state snapshots, 693 model choices, 99 RNG draws, 414 streaming queries, 18 live model mutations, four explicit initialization cases, 12 saves and 24 loads. Validation-enabled profiles see 60 returning header assertions. Hashes: default/plain/GL `4ef962d12cd3e558`; MASTER `8d54a48975af3088`; unfixed `5a72e59beff750c8`; 15-bit RNG `af56b19ca02bed13`; x86 vanilla `55e761a6a22c9df3`.
