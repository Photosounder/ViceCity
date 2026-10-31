The complete CTimeCycle owner is replaced by TimeCycleSystem.c: seven complete
loading/update/interpolation/extra-colour operations and all 62 original inline
accessors are C11 functions. All 119 original static declarations, including
the full hour/weather tables and sixteen-element sun/shadow history, have
canonical mutable C storage. The four private interpolation coefficients stay
private. The three interpolation overloads have distinct signed C names.

TimeCycleVector is a plain three-float C record. Native probes compare its size,
alignment and all component offsets with the original CVector on x86 and x64.
The original mutable sun-vector reference becomes a pointer into the actual
canonical array; it is never replaced by a temporary copy inside the update.
Existing native vector-expression callers use an explicit value conversion,
while component-only callers use the C pointer accessor directly. CTimeCycle
has no remaining class, member accessors or implicit vector constructors.

Timecycle.cpp keeps seven typed native boundaries: debug output, the complete
55000-byte shared buffer, camera blur type/setter, shadow calculation, camera
forward Z and cutscene playback. A separate native value converter supports
remaining CVector callers. Camera constants, the work-buffer extent and native
primitive signatures have compile-time proofs. Clock, weather, timer, cull-zone
and file-manager dependencies already have C interfaces and are used directly.

The complete data loader and update body are preserved. The overloaded primary
interpolation macro is expanded into the matching explicit signed operation;
the remaining extra-colour expressions retain their original evaluation and
integer conversions. Min/Max keep their repeated-operand and signed-zero
behavior. The full original CVector normalization body is translated into a C
helper with the same reciprocal square-root expression. Full unmodified native
normalization and shadow primitives are also compiled in the original/C tests.

Immutable sources, original x86/x64 type probes, exact production projections
and untouched-source digests are in build/timecycle-c-tests. Runtime comparisons
enter all 69 operations through an actual C11 caller and observe every complete
table/scalar, all four coefficients, all accessor conversions and the selected
mutable vector. Cases include static initialization, initialization retaining
unrelated state, both line endings, comments/blank separators, the fixed final
NUL path, defined partial-row retention, every weather/hour dimension, fade
directions, black extra-colour skies, signed interpolation, all history slots,
normalization underflow/overflow, and native callbacks that mutate later fields
or the history index without changing the earlier borrowed sun pointer.

Only defined original inputs are used. Invalid table indices and unterminated
or wholly malformed first rows remain unchecked as in the original. Pure clock
getters are counted without asserting an unspecified C/C++ operand evaluation
order. Sequenced native callbacks and short-circuit camera/cutscene accesses
retain their observed order. Numerical fingerprints are compared within each
exact compiler, optimization and architecture profile; existing floating-point
variation between profiles is not normalized. Quiet diagnostic executables link
their MSYS2 runtimes statically, and Windows error dialogs are suppressed.

timecycle_fixture.py audits the complete source conversion and prepares the
immutable original/C models. build/test_timecycle.py runs Clang/GCC and sanitizer
comparisons; build/test_timecycle_native.py runs optimized MSVC x86/x64 models;
build/check_timecycle_types.py compares all original declarations and signatures;
build/check_timecycle_consumers.py compiles actual consumers; build/build_timecycle.py
checks both D3D9 builds; build/check_timecycle_audits.py retains preceding source
audits. Production source and migration helpers remain stable during these runs.

Historical clock, timer, file-manager and weather producer checks additionally
prove the complete relocated time-cycle implementation. Existing byte guards
for Camera.cpp, Object.cpp, Shadows.cpp and Timecycle.h allow only the exact
whole-file time-cycle projection. The first 148-audit result is retained, and
fourteen directly rerun blocks are recorded before updating the combined log.
Six renderer fixtures retain their existing observed mock providers behind the
new C getter symbols; eight additional original/current comparisons cover
weather, traffic lights, motion-blur streaks, plane trails, clouds and background
rendering, including reflection paths. The effects-only cloud fixture also
defines the frame-counter storage referenced by its linked, uncalled horizon
code; its active original/C observations remain unchanged.
