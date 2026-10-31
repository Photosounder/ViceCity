RemoteControl.c contains both complete remote-car operations. The static-only
CRemote class and its default argument are removed. Four production caller
files use the C API; the two former no-argument release calls explicitly pass
true. The Visual Studio project registers an independently named C11 source,
retaining distinct object files for every existing C/C++ source pair.

Creation retains native pool allocation, assertion and actual automobile
construction, followed by ground placement, rotation, status, locking, road
joining and every original autopilot write. C controls assignment order and
the original chained speed/lane assignments. The world insertion precedes
the two distinct live player-vehicle lookups, player remote-slot assignment
and reference registration. Appearance queries retain short-circuit behavior,
including a second query after a non-plane first result. Aircraft use the
camera-string mode and subsequent zoom; other vehicles use behind-car mode.

Release retains the mission-to-random creation-kind transition and signed
counter changes, unlock, live timer read and all four player-state writes.
The remote slot is accessed afresh where the original expressions accessed
it. No null guard, allocation recovery, repeated-query caching, time change
or ownership-policy change was added. Native vehicle/player storage remains
canonical; the C controller introduces no shadow state or representation casts.

Thirty typed boundaries perform native construction, method calls, bitfield
writes and access to exact native named fields. C and C++ use the same opaque
CVehicle tag. Native inheritance conversions occur in C++, including the
automobile-to-vehicle conversion after construction and vehicle-to-entity
method arguments. The existing reference-registration cast remains at its
original native API boundary. Five small C constants are checked against
their original native enums with static assertions; statuses, appearance and
camera modes use the existing C-compatible production headers directly.

The source fixture compares eight entire changed source/project files and
both new production files against immutable originals. All native dependencies
retain byte guards. The older timer audit checks the entire relocated remote
algorithm as well as its current native boundary, preserving coverage when
the earlier snapshot reaches the now-removed class. The complete migration
chain forwards all updated callers and project registrations.

Runtime programs compile both complete original methods and the actual C11
controller. Both compile the current native boundary bodies. Narrow vehicle,
player and camera shells provide controllable external operations; they do
not claim to reproduce native owner layouts. They retain actual CAutoPilot,
CVector and CMatrix declarations, the real matrix rotation algorithm and
the real C pool implementation. Native runs use production common/RenderWare
types and a quiet CRT adapter for the external allocator. Independent actual
production compilations and type probes verify all owner-field types, native
inheritance, method signatures, constants and all thirty boundary signatures.

Each runtime run observes 17,920 states and 9,472 native calls, with 768 car
creations and 9,216 releases. Cases cover distinct first/second player vehicles,
no player vehicle, changes to the remote slot during native operations,
changed appearance results between queries, every aircraft/ground camera
branch, native reference-slot identity, model boundaries, signed coordinates,
ground-query arguments, both found values, both explosion flags, every vehicle
creation kind and timestamp boundaries. Road callbacks perturb autopilot
fields before the C controller writes its configuration. Checks observe all
named affected fields, all matrix components, actual pool flags and explicit
native call order. The two operands in the original ground-height addition
are counted without imposing an evaluation order the original did not specify.

Twenty Clang/GCC O0/O2 comparisons cover default, plain, MASTER, unfixed and
GL profiles. Five Clang comparisons use undefined-behavior, float-conversion
and indirect-function-type traps. Seven optimized MSVC comparisons cover
Win32 default/GL/MASTER/vanilla and Win64 default/GL/MASTER. Every comparison
has hash 6126c74e1282e404. Thirty production compilations cover the native
backend, four caller files and C11 controller across five native profiles.
Win32/Win64 type probes pass. Actual x64 D3D9 Debug and Release builds link.
Object-name checks cover all 50 configurations and all 28 C/C++ source pairs.
The preceding 133 whole-source audits are also checked.

Run utils/tests/test_remote.ps1 for portable comparisons. Add -Sanitized,
-Native, -Consumers, -BuildProject or -AuditPrevious for the respective checks.
The evidence checker is build/finalize_remote.py. These checks compile and
run console fixtures; they do not launch the game or test rendered behavior.
