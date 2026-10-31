ScriptPaths.c owns all three scripted-path records, all eighteen replay object
pointers and thirteen complete operations formerly belonging to CScriptPath and
CScriptPaths. Both classes are removed. The C record retains every field type,
offset, size and alignment: 52 bytes on Win32 and 88 bytes on Win64. Fluff.h now
contains only C-compatible aggregate includes and compiles as C11.

C owns initialization, cleanup, traversal, active-state decisions, path
interpolation, normalization, orientation, velocity, replay and serialization.
Typed native boundaries retain the actual CPlaneNode allocation/loader, CObject
inheritance and registered reference slots, matrix ownership and frame/region
publication. A real native CMatrix previous-value copy remains alive throughout
each per-object C callback, including nested updates and final velocity writes.
Object slots and time-step values are read at their original external-call
boundaries. The original angle wrapping, zero-vector fallback, direct component
division, interpolation aliases and initializer allocation behavior remain.

Both compatible and native raw saves retain their record layouts and 20-byte
native node stride. Pool handles are resolved through the actual shared C pool
implementation. Raw record assignment and native node assignment retain their
original language operations. The fixture compares every declared field and
float bit, all defined serialized bytes, record padding and live pointer slots.
Only raw native node addresses are normalized in headers. Three unspecified
tail padding bytes in each native CPlaneNode are excluded: native implicit copy
assignment may copy these bytes or leave the destination bytes unchanged, even
within one compiler configuration. They have no declared field meaning. No
production initialization or serialization was changed to make the test pass.

The fixture audits eight complete modified files and three complete new files
against immutable originals and an explicit migration. Actual native node,
matrix, vector, object, entity, pool, timer and save sources remain unchanged.
Original methods and actual matrix/vector implementations are extracted into
the C++ oracle. A separate C11 caller reaches both versions. The external owner
simulator observes all matrix coefficients, ownership metadata, registered slot
addresses, object flags, native base adjustment, path fields and callback order.

The corpus covers static zero state, all eighteen controlled slots, replay,
both save formats, negative/zero/positive speeds, time-step and clamp boundaries,
zero and oversized object lengths, aliased coordinate outputs, sustained motion,
changed live positions/slots/time steps, registration before activation, loader
output aliases, reinitialization, pool changes during loading, later-record
activation, freeing callbacks and nested matrix lifetimes. Original unfreed
storage on repeated initialization is observed before harness cleanup.

Twenty-four Clang/GCC O0/O2 comparisons cover default, plain, reflection, GL,
unfixed and raw-save profiles. Six Clang comparisons use undefined-behavior and
float-conversion traps. Seven optimized native MSVC comparisons cover Win32
default/GL/raw/vanilla and Win64 default/GL/raw. Native type probes verify every
field, record/node layout, public controller signature, typed owner boundary
and C11 aggregate header. Sixty-five production compilations cover twelve C++
consumers plus the C kernel across five architecture/feature combinations.
Actual Debug and Release x64 D3D9/OpenAL projects compile and link. Fifty project
configurations retain distinct objects for all twenty-eight paired C/C++ units.
All 123 preceding source audits pass. No game or GUI test was launched.

Run utils/tests/test_script_paths.ps1. Optional switches select Sanitized,
Native, Consumers, BuildProject and AuditPrevious. Immutable originals,
generated oracles, logs and result manifests live in build/script-paths-c-tests.
