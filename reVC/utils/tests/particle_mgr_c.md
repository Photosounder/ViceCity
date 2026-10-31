ParticleConfig.c owns the complete particle configuration parser, initialization,
all 83 configuration records and the mutable ParticleFilename pointer. The
cParticleSystemMgr class, its runtime zeroing constructor and its unused empty
RangeCheck method are removed. C static initialization supplies the same zero
record bytes. The only remaining ParticleMgr.cpp operation borrows the original
55,000-byte work buffer from main; it does not own or duplicate that storage.

Every original record field keeps its offset, size and alignment. Records remain
148 bytes on Win32 and 160 bytes on Win64. CRGBA fields are plain four-byte C
color unions with the original four-byte alignment and named channel offsets.
CVector2D texture stretch is a plain two-float C value. Particle and raster slots
remain typed opaque pointers; native C++ sees the exact original CParticle *
and RwRaster ** types. The two native particle color transfers now explicitly
copy four channels before the original RwRGBAAssign operation.

The original parser's line scanning, exact end marker, comment handling, strtok
state, 41-column ordering, parameter wrap, numeric conversions, unsigned and
signed narrowing, variation cap and squared creation range are retained. Direct
reload retains every field not present in input, including particle pointers.
Initialization loads data first, then clears all 83 particle-list pointers while
retaining raster pointers and unrelated data. File-owner call order, live
filename reads, shared-buffer reads after directory restoration and the ignored
file-load result remain unchanged. Malformed input handling was not altered.

The fixture audits five complete modified files and both new files against
immutable originals and the explicit migration. Native common, vector, file,
particle type, particle class, RenderWare color and main buffer sources remain
unchanged. The original manager declaration, constructor and full algorithms
compile in the oracle. A separate C11 caller reaches both versions. Both actual
renderer color-transfer statements are extracted from their production files
and compiled in the comparison, including the original CRGBA conversion.

The corpus compares every record byte, retained padding, named color channel,
canonical pointer identity, mutable filename and all 55,000 shared buffer bytes.
Only actual process pointer addresses are normalized. It covers every record
and column, full and short rows, extreme defined integer inputs and narrowing,
finite float rounding, signed zero, invalid numeric tokens, negative/capped
variation, range squaring, maximum valid line length, parameter wrap, trailing
whitespace/CR, comment-only files and reload versus initialization. Owner hooks
replace the live filename, return a negative load result with available input,
replace the shared buffer and retained fields, and reenter loading before the
outer parser obtains its cursor. Both native color sites execute 20,086 times
per program run.

Twenty Clang/GCC O0/O2 comparisons cover default, plain, MASTER, unfixed and GL
profiles. Five Clang runs enable undefined-behavior and float-conversion traps.
Seven optimized native MSVC comparisons cover Win32 default/GL/MASTER/vanilla
and Win64 default/GL/MASTER with actual project headers. Fail-fast stubs cover
four unused interpreter dependencies pulled in by those headers; any call to
them aborts. Native type probes check all 39 fields, full record/pool layout,
color/stretch representations, actual pointer aliases, buffer extent and a
standalone C11 header.

Twenty-five production compilations cover ParticleMgr.cpp, Particle.cpp,
ParticleObject.cpp, WaterLevel.cpp and ParticleConfig.c across five architecture
and feature combinations. Actual Debug and Release x64 D3D9/OpenAL builds
compile and link. Fifty project configurations retain unique object paths for
all twenty-eight paired C/C++ units. All 124 preceding source audits pass.
No game or GUI was launched.

Run utils/tests/test_particle_mgr.ps1. Optional switches select Sanitized,
Native, Consumers, BuildProject and AuditPrevious. Immutable originals,
generated inputs, logs and result manifests are in build/particle-mgr-c-tests.
