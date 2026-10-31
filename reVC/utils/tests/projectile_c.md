# Projectile controller in C11

`src/weapons/ProjectileControl.c` owns all 32 projectile records, their 32 native projectile pointers, the optional active counter and the four original function-static smoke arrays. It implements all eleven original controller operations. `ProjectileInfo.h` exposes a plain record and C functions; replay still copies the same arrays with the same record stride (28 bytes on x86, 40 on x64). The position packet is twelve bytes, and all original field offsets and types are checked against the native original header.

Nine complete callers use the C API. Creation and `RemoveNotAdd` accept an opaque pointer to the caller's native vector and immediately capture its original value. The current callers pass the same local/source lvalues they previously copied. No C++ constructor or reference is required by a C consumer.

`ProjectileInfo.cpp` provides typed adapters for entity fields, matrix operations, particle/world calls and native construction/destruction. Every controller choice remains in C: launch branches, numeric velocity/elasticity calculations, first free record selection, object-pool eviction order, source validation, lifetime checks, collision short-circuiting, range filtering and removal. The real C pool API is called directly from C. A stack-scoped native matrix surrounds the complete creation operation, preserving its original lifetime on every return. Temporary particle/explosion vectors are consumed synchronously by their native calls; live position references used by tear gas stay live across particle callbacks.

The original smoke extension calls `Timer_GetTimeStepFix` even when `FIX_BUGS` hides that helper. The C controller supplies the identical two existing timer helper bodies locally in that configuration. The unfixed/vanilla C++ comparison oracle is supplemented with those same complete helper bodies from the immutable timer header; its controller implementation is unchanged. The fixture checks that the local C helper bodies match the originals after ignoring comments/formatting.

Existing behavior is retained, including the post-increment removal index in `RemoveIfThisIsAProjectile`, retained pointer slots after most removals, optional-counter behavior, shutdown's lack of cleanup and the original positional argument `3000` to `AddParticlesAlongLine` (rotation, with default frame/lifespan). Tests keep the original valid-input preconditions: supported weapon types and valid required sources; they avoid the undefined final-slot match in the post-increment lookup, full-capacity scanning without `FIX_BUGS`, and construction from a null allocation in that old configuration.

Run from the repository root:

```powershell
./utils/tests/test_projectile.ps1 -Native -Consumers -BuildProject -AuditPrevious
./utils/tests/test_projectile.ps1 -Sanitized
```

The fixture compiles the entire immutable original controller and header as its oracle. Both variants use actual `Pool.h`, `General.c`, vector and matrix operation bodies, timer accessor bodies and the original `CProjectile` constructor. Thin native entity/world/particle shells supply deterministic external state. The same current native adapter source is compiled for comparisons, and actual production callers are separately compiled against the real project headers on x86/x64. Native probes verify record offsets, pointer targets, replay arrays and unchanged function signatures; all opaque vector pointers target the actual native `CVector` class.

The corpus independently asserts coverage of all five weapons, all five rocket launch branches, signed/zero speeds, all four collision short-circuits, all four boundaries, molotov source-distance gating, live gas-position mutations between calls, timer changes between particle calls, sub-frame and multi-frame smoke, source validity, expiration, detonator weapon/ammo lookup, first/last TEMP_OBJECT eviction, allocation failure, full record capacity, inclusive range queries, shutdown and the retained removal-index behavior. A fresh cycle retains the original smoke static state rather than resetting it for tests. No game/window or rendering device is launched.

Validation comprises 24 Clang/GCC original/C comparisons (O0/O2, six profiles), nine MSVC comparisons (x86/x64, D3D9/GL/MASTER/SQUEEZE plus x86 vanilla), six sanitizer comparisons, 55 actual production compilations, both actual D3D9 Debug/Release links, all 50 evaluated configurations with unique objects, and 137 preceding whole-source audits. Earlier timer/general snapshots additionally prove the complete relocated controller.

The default corpus reports 817 snapshots, 502 constructions/destructions, 108 explosions, 201 gas particles, 55 smoke segments, 20 choking calls, 106 visibility queries, 49 helicopter queries and 48 plane queries. Default hash: `9709803a1032aada`. The optional-counter profile has 818 snapshots and hash `0444400a55d49bfd`. Unfixed/vanilla profiles omit the undefined allocation/capacity cases: 813 snapshots, 308 constructions/destructions and hash `cbaac4e94f957a59`.
