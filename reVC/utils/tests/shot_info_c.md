# ShotInfo C ownership

`src/weapons/ShotInfo.c` implements all four original operations, the complete
100-record shot array, and its twenty-entry random table. Shot selection,
expiry, movement, ped filtering, fleeing, fire propagation, and staggered vehicle
fire decisions live in C. The weapon caller copies its original by-value native
vectors into genuine C vector records.

The native unit accesses separately owned entities and calls the existing
`CVector::Normalise` operation on an actual temporary native vector. It forwards
diagnostics and assertion failures. It contains no shot selection or propagation
loops. The project assertion expression still evaluates in MASTER builds.

Original details remain intact: invalid source validation precedes inactive-slot
skipping, expired shots finish their current iteration, z spread is unscaled,
squared distance uses the original unsquared radius threshold, and ped lists and
source fields stay live across callbacks. The optional SQUEEZE_PERFORMANCE
counter retains its zero-count early return. Empty original vector default
construction leaves global zero initialization unchanged.

## Evidence

`.diagnostics/shot-info-c-tests` retains the complete original and staged source
trees and these checks:

- 40 strict C/public-interface compiles
- 24 complete SDK owner compiles, including the optional active counter
- Two complete native weapon-consumer compiles
- Actual SDK and independent C/C++ layouts on both ABIs
- 159 complete historical source projections
- 40 whole original/C behavior comparisons on both ABIs, two optimization
  settings, and ten feature profiles
- 48 evaluated native project configurations
- Complete D3D9 Debug and Release links

Behavior comparisons fill all shot slots, reject a full array, test both expiry
boundaries, exercise invalid and inactive sources, filter nearby peds, and change
the source pointer, time step, and near-ped list during callbacks. A fire callback
reenters complete shot allocation. The runtime uses complete original/C owners,
actual metadata records, the original vector header and normalization body, and
the exact staged native unit. External entity and fire operations are controlled
observations. These checks do not run the game or establish gameplay behavior
outside the tested boundaries.

Diagnostic executables use the static MSVC runtime and Windows-only execution
PATH. All checks stay under `.diagnostics` without recreating `build`.

## Applied-source checks

```text
utils/tests/shot_info_native_sources.py --production
utils/tests/shot_info_native_consumers.py --production
utils/tests/shot_info_history.py --production
utils/tests/weapon_effects_stage_audit.py --production --integration
utils/tests/sprite2d_stage_audit.py --production --integration
utils/tests/sprite_stage_audit.py --production --integration
utils/tests/shot_info_stage_audit.py --production --integration
```

Run the scripts with the installed Python interpreter. The final audit requires
exact applied candidate bytes, current production compiler results, all runtime
traces, full integration evidence, and preceding audit records before reporting
`applied-verified`.
