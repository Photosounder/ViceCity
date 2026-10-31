# Fire lifecycle in C11

`src/core/FireSystem.c` owns the canonical `gFireManager`, its forty fire records and all seventeen original record/manager operations. The `CFire` constructor and empty destructor have been removed. The records and manager are plain C data; constructor-equivalent static initializers preserve all original startup values, including the next-flame timer previously supplied by static zero-initialization. The twenty-three complete caller files use the C API.

`Fire.cpp` now contains fifty-five typed native field/action adapters and two native vector conversions. Fire allocation, rejection rules, damage amounts, expiry, particle timing, random choices, script reservations, water effects, queries and update order are in C. Reference registration calls the existing `References_RegisterReference` C API using the actual record field addresses. Point-light registration enters the existing C API directly. Native actors, particles and shadows remain behind typed adapters until their own conversions.

Before editing, native x86/x64 probes established the actual record/manager sizes: 48/1924 bytes on x86 and 56/2248 bytes on x64. The C data preserves every field offset/type, vector component representation, alignment and the complete manager array representation used by replay. `FireVector` is a plain twelve-byte value. The script position retains an opaque pointer to the actual native `CVector`, because the original operation reads its referenced position only after extinguishing an existing target fire. A comparison case mutates that position during pedestrian cleanup and checks that script creation observes the mutation.

Ordinary creation retains a reused slot's water countdown; script creation resets it to one. Extinguishing retains the source field and reference registrations, clears the target and ongoing flag, and decrements the ordinary counter only for non-script fires. An extinguished script slot remains reserved until script removal clears its script flag. These original policies, strict range/time comparisons, first-match water behavior and callback/write order are retained. The script API keeps its original valid-index and free-slot preconditions.

Run from the repository root:

```powershell
./utils/tests/test_fire.ps1 -Native -Consumers -BuildProject -AuditPrevious
./utils/tests/test_fire.ps1 -Sanitized
```

The quiet console oracle compiles the complete immutable original constructor, destructor, methods and canonical manager. Both variants link the real `General.c`, `References.c`, native vector/matrix code and original point-light adapter. Native actor/effect shells observe arguments, callback ordering, live reads, damage and reference chains. Independent assertions stay enabled in MASTER. No game or graphical test is launched.

Coverage includes all seventeen operations; all forty startup records; 113 point/entity creation cases; ten script creations; first-free selection and a fully reserved array; seven attachment categories across twenty processing scenarios; player ignition and processing in the same update; vehicle headlights transformed by the actual matrix code; expiry equality; negative steps; NaN water countdown; fractional water thresholds; first-match water effects; nearest/furthest ties and strict ranges; live source/target reference resolution; script reservation cleanup; pedestrian state gates; repeated extinguishing; and unsigned counter underflow inherited from the original implementation.

Validation passes 24 Clang/GCC original/C comparisons at O0/O2 across six profiles, nine native MSVC comparisons on x86/x64, six sanitizer comparisons, 110 compilations against actual production headers, complete native/C layout checks, both actual D3D9 Debug/Release links, unique object paths in all fifty configurations, and 139 preceding whole-source audits. Earlier snapshots also prove the complete relocated fire kernel and native adapters. The first audit run's diagnostics are retained separately; the final run passes all 139 audits.

Default comparisons observe 344 whole-state snapshots, 774 random draws, 479 native actions, 387 events, 250 particles, 26 shadows, 123 light registrations and 407 reference predicates. The unfixed/vanilla branch observes 343 states, 480 actions, 386 events and 25 shadows. Each original/C pair matches exactly within its compiler and configuration. Clang and GCC/MSVC differ in the original unspecified evaluation order of three random constructor arguments for steam; cross-compiler hashes are consequently distinct.

| Profile | Clang hash | GCC/MSVC hash |
| --- | --- | --- |
| Default/plain/MASTER/GL | `65c31d42b2f75621` | `69cf3449175e6099` |
| Unfixed | `aa148f90b57b2892` | `c4ff42c9bff9d13a` (GCC) |
| 15-bit RNG | `69f46e3d2e947532` | `b8f28ecd838e80fe` |
| x86 vanilla | — | `22c13c6f77127e31` (MSVC) |
