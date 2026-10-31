WeatherSystem.c contains all twelve former weather methods, the complete
billboard helper and timestep helper. CWeather is removed. All 24 public
scalars, both 64-entry weather schedules, the wind table and all 35 rain
records are canonical C data. The ten original file/function static values
remain in their original scopes with their original initialization and
lifetime. Thirty-six production caller files use the C API and variables.

The controller preserves interpolation, weather forcing, schedule selection,
fractional update accumulation, lightning probabilities/timing, sound and pad
arguments, road wetness, clouds, fog, rainbow, sun glare, wind and traffic-light
brightness. Particle decisions retain no-rain and roof returns, steam timing,
day boundaries, hurricane splashes, drop counts, native overload/default
arguments and all random draws. No range clamp, timer reset or behavior fix
was introduced. The guarded SECUROM operation remains guarded and changes
both schedules and all three original weather selections.

Rain rendering retains every original vertex, UV coordinate, color, index,
triangle winding, distance fade and native render-state write. C performs
the original component arithmetic and vector normalization. Reflection passes
retain the selected auxiliary camera basis and draw existing streaks while
skipping generation, expiration writes and legacy random side effects. Primary
passes retain those original changes. Native transform failure skips indexed
rendering/end while preserving the unconditional state restoration and final
buffer resets. The existing C render buffer remains the only buffer storage.

Fifty-three typed boundaries retain native camera, player, particle, collision,
pad, timecycle and RenderWare access. Native owner layouts are not assumed.
Value packets transfer named components into actual native vectors/colors.
Inspection of particle/collision code confirms inputs are consumed or copied
synchronously. The rain texture remains owned by the particle module with
its original native type and C++ linkage. Render constants are checked against
native enums. Rain records remain 28 bytes with four-byte alignment; vector
and color packets retain the actual 12-byte/four-byte representations.

The source fixture compares forty complete changed source/project files and
both new production files with explicit transformations of immutable originals.
Native dependencies retain byte guards. Earlier clock, timer, cull-zone,
general, special-effect and surface audits verify the complete relocated
weather algorithms as well as the current backend. Earlier comparisons apply
the explicit weather spelling change while comparing every other statement.
Dependency guards retain exact bytes unless the entire saved file matches
the explicit weather transformation. Initial audit failures remain recorded.

Legacy audio migration helpers retain both intermediate weather spellings
when locating adapter blocks and binding original C++ fields to existing C
host calls. Existing outer edit markers are reused so separately generated
adapter blocks remain identifiable. These are source-audit compatibility
changes; the audio kernels and game behavior are unchanged.

Runtime programs compile the complete unchanged original weather source and
the actual C11 kernel. A separate C11 caller enters all thirteen public
operations, including the billboard helper and guarded hurricane operation.
Both programs link actual General.c, RenderBuffer.c/RenderBuffer.cpp, complete
actual clock getter bodies, actual native vector normalization and actual
typed vertex/index storage. Native runs also compile the unchanged external
fast_absf body required by the original MSVC math wrapper. Narrow camera,
player and particle shells control foreign operations without claiming to
reproduce their native owner layouts. Actual native production compilations
and type probes independently verify those bindings.

The corpus covers every weather pair, clock/replay/debug gates, fractional
steps, lightning/audio/shake scheduling, glare random suppression during
snapshots and daylight rainbow boundaries. It covers rain exclusion, roof
returns, steam/day conditions, target fallback, collision hit/miss, particle
defaults and callbacks that change later live inputs. Streak tests include
all lifetime/fade boundaries, zero directions, reflection frame/matrix absence,
transform failure and changed live buffer counts after transformation. The
two-second streak case checks legacy unfixed random side effects. Direct
billboard tests cover scale, pause state, buffer offsets and distance limits.
Every persistent scalar/table component and every logical vertex/index value
is compared; private padding and process addresses are excluded.

There are 24 Clang/GCC O0/O2 comparisons across default, plain, MASTER, unfixed,
GL and SECUROM profiles; six Clang sanitizer comparisons use undefined-behavior,
float-conversion and indirect-call traps. Eight optimized MSVC comparisons
cover Win32 default/GL/MASTER/vanilla/SECUROM and Win64 default/GL/MASTER.
Default runs observe 677 states, 440 updates, 144 streak passes, 24 direct
billboards, 29,987 random draws and 8,652 particles. Win32 and configuration
hashes may differ from Win64; each original/C pair must match exactly.

Native checks compile 190 actual production units across five profiles and
check all scalar types, record offsets, public signatures, native particle
overloads and all 53 boundaries on Win32/Win64. Actual x64 D3D9 Debug and
Release builds link. Object paths remain unique in all 50 configurations.
The preceding 135 complete-source audits are also checked.

Run utils/tests/test_weather.ps1 for portable comparisons. Add -Sanitized,
-Native, -Consumers, -BuildProject or -AuditPrevious for the respective checks.
The evidence checker is build/finalize_weather.py. All tests are quiet compiler
and console checks; they do not launch the game or create a graphics window.
