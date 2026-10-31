# Weapon metadata in C11

`src/weapons/WeaponData.c` owns all 37 canonical weapon records, the reload-sample and maximum-ammunition tables, and the private 37-by-32 name table. All eight original operations, including the two inline predicates, are C functions. `CWeaponInfo` is now a plain C record with no methods or implicit native-vector construction. Twenty-four complete caller/header files use the C API, explicit record pointers and native offset values.

Native probes before editing established the actual record size as 100 bytes, aligned to four bytes, on x86 and x64. Checks against the complete original class preserve every member type/offset, vector representation, public table type and function signature. Global record bytes still start at zero. The former empty native-vector construction adds no startup state. Both mutable public tables retain their exact original initializers and stay unchanged by initialization/loading.

The conditional animation-group enum is shared in `src/animation/AnimGroups.h`. Its complete original declaration, order and values are retained, including both the 61-group player-control layout and the 52-group classic layout. Native animation owners include the same canonical enum. C headers compile independently; C++ callers retain their existing animation-header dependencies.

`WeaponInfo.cpp` contains ten typed native operations for the exact diagnostics, shared work buffer, shotgun predicate, model registration and animation-group names, plus one native vector conversion. All parsing, lookup decisions, resets, firing-rate calculation, model substitutions, animation selection and predicates remain in C. The actual shared 55,000-byte native array is borrowed through a pointer to its full array type; no replacement buffer or duplicate owner is introduced.

Initialization retains its partial reset: model IDs survive, while the original reset fields receive their original defaults. Loading keeps the existing row format, whitespace/comments, sentinel-prefix behavior and case-sensitive fallback rules. Animation names select the first matching group; an unknown name retains the prior group. Firing rate is recomputed only under the original strict frame threshold and weapon exclusions. Registration uses the local model substitution for detonator grenades without changing the record's parsed model ID, and skips detonator/helicannon registration. The original token/line sizes, valid indices and numeric-conversion preconditions are retained.

Run from the repository root:

```powershell
./utils/tests/test_weapon_info.ps1 -Native -Consumers -BuildProject -AuditPrevious
./utils/tests/test_weapon_info.ps1 -Sanitized
```

The quiet console oracle compiles the whole immutable original class, records, table initializers and parser. It uses the real vector declarations and complete native model setter and animation-name getter, with instrumentation around those operations. Model shells force a nonzero base adjustment and retain prefix guards. File/directory endpoints supply deterministic bytes through the existing C API and actual shared array; filesystem behavior is covered by the maintained file-manager comparisons. Assertions remain active in MASTER. No game or graphical test is launched.

Coverage includes all 37 startup records and stable addresses; exact-case and unknown lookups; all fire categories and error text; 6,290 flag cases across every bit, composite masks and zero masks; 101 slot predicates; 37 native offset conversions; partial/repeated initialization; public-table persistence; complete rows for every weapon; comments and CRLF; a final line without newline; sentinel prefixes; strict firing-rate thresholds; missing animation names; special model IDs; duplicate rows; and native callbacks changing later canonical fields/name reads.

Validation passes 24 Clang/GCC original/C comparisons at O0/O2 over six profiles, nine native MSVC comparisons on x86/x64, six sanitizer comparisons, 161 actual production compilations, field/signature/table/buffer/enum checks in both architectures and control layouts, both actual D3D9 Debug/Release links, unique object paths in all fifty configurations, and 142 preceding whole-source audits. The earlier file-manager snapshot also proves the complete relocated loader and native bindings. The rope audit follows the exact shared-enum header transformation; first-pass diagnostics are retained separately.

Each comparison reports 22 whole-state snapshots, nineteen loads/directory calls, thirteen diagnostics, 497 model registrations, 6,290 flag cases, 101 merging-slot cases, 42 named type cases, eight fire-category cases, 37 native offsets and two explicit initializations. The 61-group layout records 14,605 animation lookups and hash `4f092c980b9799aa`; the classic layout records 13,903 lookups and hash `96660d184f2e0eab`. Original and C results match in every compiler/profile.
