The bullet-trace owner class and record member method are removed from
SpecialFX.h. BulletTraces.c owns all six original algorithms, the public
sixteen-record pool, the complete 48-entry triangle table, shared UV setup and
smoke-trail resource orchestration. The two AddTrace overloads become explicit
Add and AddWeapon C functions, each with typed C-vector and legacy-pointer input
paths. The same C implementations drive both paths.

Actual MSVC Win32/Win64 probes verify 44-byte records aligned to four: start
at zero, end at twelve, active flag at 24, creation/lifetime at 28/32, thickness
at 36 and visibility at 40. Indices are uint16_t and capacity remains sixteen,
including the original literal nextSlot < 16 gate. Native math symbols select
fast_absf and sqrtf; original vector division, normalization, dot products and
scalar multiplication order remain in C.

All input reads remain live, including position-then-end assignment aliases and
subsequent sound projections. Camera vectors are borrowed through actual typed
references and read at their original logical use. The C projection helper
preserves the observed Clang versus GCC/MSVC argument order, including references
affected by another getter. Typed owner conversions preserve entity, vehicle and
physical-base pointer adjustments. Repeated player/vehicle queries, camera-mode
gates, speed threshold and all weapon presets execute in C.

CameraModes.h is the authoritative plain C enum for every original camera mode.
The existing CCam names remain scoped aliases to those constants. Both native
probes compare every alias with the original enum value. No camera data member
or method changes. WeaponType.h and DMAudio.h are already C-compatible and are
used directly by the C implementation.

Rendering retains main-camera reads before pool traversal, every reflection
camera/frame/matrix null branch, repeated matrix queries, camera-relative
clipping, lifetime/alpha arithmetic, three distinct geometry passes and both
FIX_BUGS coordinate variants. Ten actual typed vertices remain in the C++
backend; C controls every field write and all 48 indices. The original literal
first-pass flags and symbolic UV flags remain distinct at their typed boundary.
Failed transforms, final state restoration and shared-buffer reentry retain
their original behavior. Thirty-six backend operations expose actual game/RW
owners without guessing layout; replay value conversion is explicit.

Manager initialization still clears only active flags. UV initialization still
writes only the original fifteen fields, leaving V for vertices five through
nine untouched. Texture push/find/set/conditional-read/pop remains at its exact
shared-effect initialization point. Destruction retains the published handle
until the owner returns and then clears it. Resource setup and teardown bodies
are extracted completely from the original source.

Replay's existing 28-byte packet stays unchanged. Exact original and current
record-to-packet assignments and the three partial playback writes participate
in runtime comparisons. Playback continues to leave creation time, lifetime,
thickness and visibility untouched. Whole replay source transformations and
actual production compilation independently check all users.

bullet_traces_fixture.py audits all eight entire changed source/project files,
complete algorithms, exact resource/geometry setup and shared camera constants
against immutable originals. The oracle compiles complete original methods and
setup statements. Real vector methods, math wrappers and typed vertex fields are
retained. Separate C11 callers exercise actual C APIs; real subobject aliases use
the C and C++ input paths. Native comparisons use actual project/RW headers and
legal mock owners with nonzero base-address offsets.

The corpus compares 622 complete pool/resource/geometry snapshots and 3,000
mixed frames. It covers every occupancy count, both lifetime-density thresholds,
full-pool sound generation, actual input aliases, all camera modes and weapon
presets, player/vehicle identities, speed-boundary rejection before null input
reads, unsigned time wrap, zero lifetime and exact expiry, replay partial writes,
reflection null branches, near clipping, zero-length traces, every draw outcome,
texture failures/retries, dictionary restoration and nested rendering/destruction.
Undefined render-time alpha conversions are excluded; zero-lifetime cases are
tested through registration/expiry. Finite float bits compare exactly and NaNs
by classification. Fingerprints retain original compiler-specific evaluation
order differences.

Sixteen Clang/GCC O0/O2 comparisons and four undefined-behavior trap comparisons
cover default, non-librw, GL3 and unfixed-coordinate profiles. Five MSVC/LTCG
comparisons cover native Win32/Win64 D3D9/GL3 plus vanilla Win32. Thirty actual
production compilations cover the backend, four callers and C unit across those
native configurations. Debug and Release x64 D3D9/OpenAL projects compile/link
with existing dependencies and Git-hash events disabled. No game is launched.
All fifty project configurations retain unique objects, now including nineteen
C/C++ pairs. Commands, immutable inputs and results are in build/bullet-traces-c-tests.
All 104 preceding whole-source migration audits pass. The final evidence is
checked by build/finalize_bullet_traces.py; test_bullet_traces.ps1 exposes the
portable, native, sanitizer, consumer, project and source-audit checks.
