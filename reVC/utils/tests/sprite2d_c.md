# Sprite2d C ownership

`src/renderer/Sprite2d.c` implements all 29 original operations, including the
constructor, destructor, inline vertex getter, and every overload. Its public
record contains one opaque texture pointer. `Sprite2d_InitArray` and
`Sprite2d_DeinitArray` replace implicit array construction and destruction.

The native unit owns the actual eight SDK vertices and forwards individual SDK
operations. The C owner borrows the existing immediate render buffer and reuses
the preceding Sprite, Shadow, and RwHelper C boundaries. Rendering policy,
texture replacement, vertex assembly, and buffer flushing remain in C.

Consumers use C functions and explicit live colour/rectangle views. Cleanup is
registered at each of the 45 original global definition boundaries. The HUD
local retains its first-call static guard. Both menu layouts initialize their
embedded records and destroy them after the original parent shutdown body. The
PS2 configuration without FIX_BUGS still destroys its embedded sprites.

## Evidence

The complete immutable baseline and coherent candidate are under
`.diagnostics/sprite2d-c-tests`. Checks include:

- 40 strict C and independent public-interface compiles
- 18 complete owner compiles against actual SDK headers
- 32 complete consumer compiles on both Windows ABIs
- Six actual SDK/menu ABI comparisons from COFF data
- 40 whole original/C operation comparisons across both ABIs, two optimization
  settings, and ten feature profiles
- Four extracted global/HUD lifetime comparisons, each allocating and destroying
  130 textures with an independent reverse-destruction expectation
- Twelve menu lifetime comparisons using the exact original and staged
  destructor and UnloadTextures bodies, with independent destruction-order
  expectations for six scenarios
- 1,377 complete historical source projections
- 48 evaluated native configurations and complete D3D9 Debug/Release links

The menu runtime containers isolate sprite member lifetimes; they do not execute
the unrelated full parent constructors. The separate ABI checks compile complete
actual parent declarations. Runtime SDK operations are controlled observations;
these checks do not establish visual output on a GPU. The game is never launched
by these tests.

Native diagnostic executables use the static MSVC runtime and a Windows-only
execution PATH. Checks live under `.diagnostics` and do not recreate `build`.

## Applied-source verification

Run with the installed Python interpreter:

```text
utils/tests/sprite2d_native_sources.py --production
utils/tests/sprite2d_native_consumers.py --production
utils/tests/sprite2d_history.py --production
utils/tests/sprite_stage_audit.py --production --integration
utils/tests/shadow_cameras_stage_audit.py --production --integration
utils/tests/rw_helper_stage_audit.py --production --integration
utils/tests/sprite2d_stage_audit.py --production --integration
```

The stage audit requires exact applied candidate bytes, compiler results,
historical projections, lifetime traces, both linked executables, and the
preceding owner audit records before reporting `applied-verified`.
