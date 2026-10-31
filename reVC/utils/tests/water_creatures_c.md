SeaLife.c owns the eight canonical WaterCreature records, their disabled static
initialization, the signed active counter, all 65 property rows and all twelve
record/controller operations. Both CWaterCreature and CWaterCreatures are gone.
The C public header contains plain records and an opaque native position type.
Game, replay and water-level call sites enter the C operations directly.

All property model pointers refer directly to the canonical C model-ID globals.
No binding constructor, shadow property table or copied model registry is used.
The records retain the original named field types, offsets and native sizes:
24 bytes on Win32 and 32 on Win64. Property rows remain 20 and 24 bytes.

The C code retains every allocation, state transition, alpha adjustment, random
draw, heading calculation, velocity calculation and removal decision. It uses
the actual C pool implementation, including saved-handle recovery in FIX_BUGS
builds. Original strict distance/depth boundaries, unsigned lifetime arithmetic,
signed count updates and live reads across external callbacks remain unchanged.
The unfixed allocation failure bug is preserved; its invalid null construction
path is not injected into unfixed test profiles.

Twenty-eight typed C++ boundaries retain operations on native objects, matrix
publication, inherited entity methods, world membership, camera, player and
water-depth owners. A native CVector fish-position copy stays alive throughout
the C creation callback, and the same actual object is borrowed by camera and
depth calls. Updates borrow the actual object position. C only sees opaque
native position pointers or explicit three-float value packets; it never casts
a C packet to CVector. Both languages use the same CVector tag for the opaque
type, including callback signatures checked by the function-type sanitizer.

The fixture compares seven complete modified source/project files and both new
production files with immutable originals and the explicit migration. Earlier
General and timer audits also verify the entire relocated algorithm when their
snapshots contain the old WaterCreatures.cpp implementation. Their proof covers
the complete C kernel as well as the current native boundary file.

The original and C console programs use the actual unchanged vector normalizer,
matrix construction/rotation/publication code, General.c, original RNG and C
pool implementation. They link the actual canonical ModelIndexData.c storage.
The native simulator gives the inherited entity base a nonzero offset and
checks lifetime, pool generations, all declared record/property fields, native
object fields, matrix coefficients, published RW matrices and external call
order. Unspecified record padding is excluded; every named field is compared.

The corpus covers every property row, randomized invalid model selections, all
five slot states, both alpha step sizes, zero-vector normalization, strict
distance/depth boundaries, lifetime wrap, visibility rejection, failed depth
queries, temporary-object limits, full pools, saved-handle reuse and an input
position aliased into a reconstructed object. Mutation hooks change the caller
position before its copy, the borrowed fish position during visibility, model
properties after RNG, slot availability after alpha publication, player state
during an earlier slot update, the current record pointer during depth lookup,
the global pool during recovery and the active count during destruction.
Sustained updates exercise fading through deferred removal.

Twenty Clang/GCC comparisons cover O0/O2 and default, plain, MASTER, unfixed and
GL profiles. Five Clang runs enable undefined-behavior and float-conversion
traps, including indirect function-type checks. Seven native optimized MSVC
comparisons cover Win32 default/GL/MASTER/vanilla and Win64 default/GL/MASTER.
Native probes verify complete record/property layouts, field types, all twelve
C signatures, all native contracts and a standalone C11 public header.

Twenty-five production compilations cover the four actual C++ consumers and
the C unit across five architecture/feature combinations. Actual Debug and
Release x64 D3D9/OpenAL projects compile and link. All fifty project
configurations retain distinct object paths for the twenty-eight paired C/C++
units. All 128 preceding source audits pass. No game or GUI test is launched.

Run utils/tests/test_water_creatures.ps1. Optional switches select Sanitized,
Native, Consumers, BuildProject and AuditPrevious. Immutable originals, generated
inputs, logs and manifests are in build/water-creatures-c-tests.

The sanitizer wrapper explicitly builds an argument array. Assigning a single
conditional string and splatting it caused this PowerShell host to pass each
character separately, silently selecting ordinary tests instead. The same
pattern was corrected in 38 earlier wrappers. sanitizer_arguments.ps1 verifies
the actual Python arguments in both modes of all 39 affected wrappers without
rerunning the corresponding compiler matrices. Earlier direct sanitizer runs
remain valid; this correction fixes the optional wrapper entry point.
