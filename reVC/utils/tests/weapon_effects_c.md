# WeaponEffects C ownership

`src/weapons/WeaponEffects.c` implements the original five static operations
and two empty object lifetime operations. `WeaponEffects` and its position are
genuine C records. All four consumer translation units call the C operations;
the player caller copies its original by-value vector into the C record.

The native unit queries the actual current player weapon and translates SDK
constants. Texture creation, destruction, raster access, and render-state writes
reuse the previously converted C peer boundaries. Rendering decisions and the
complete weapon size table remain in C.

Preserved behavior includes ignored mark colours, the unused stored size field,
strict rotation wrapping, projection-failure exits, the FIX_BUGS blend choice,
and unguarded texture shutdown. Both original lifetime bodies and the original
vector default constructor are empty; the source audit checks their exact
absence of work against the explicit C lifetime functions.

## Verification

Evidence lives under `.diagnostics/weapon-effects-c-tests`:

- 40 strict C and independent public-interface compiles
- 18 complete actual SDK owner compiles and eight complete consumer compiles
- Complete actual SDK, C, and C++ record layout comparisons on both Windows ABIs
- 40 complete original/C runtime comparisons across both ABIs, two optimization
  settings, and ten feature profiles
- 312 complete historical source projections
- All 48 native project configurations evaluated
- Complete D3D9 Debug and Release links

Runtime comparisons cover all 37 valid weapon-table entries, 84 draws, failed
projection and texture creation, rotation wrapping, nested drawing, late owner
mutation, ignored colours, inactive rendering, and diagnostic group boundaries.
They compile complete original and C owners and four exact existing native peer
definitions, with controlled external SDK and Sprite observations. They do not
run the game or prove GPU visual output.

Executables use the static MSVC runtime and a Windows-only execution PATH. All
checks stay under `.diagnostics`, allowing independent cleanup of `build`.

## Applied source checks

Run these scripts with the installed Python interpreter:

```text
utils/tests/weapon_effects_native_sources.py --production
utils/tests/weapon_effects_native_consumers.py --production
utils/tests/weapon_effects_history.py --production
utils/tests/sprite2d_stage_audit.py --production --integration
utils/tests/sprite_stage_audit.py --production --integration
utils/tests/shadow_cameras_stage_audit.py --production --integration
utils/tests/weapon_effects_stage_audit.py --production --integration
```

The final audit requires exact applied source bytes, current compiler results,
all original runtime traces, both linked builds, and complete preceding audit
records before reporting `applied-verified`.
