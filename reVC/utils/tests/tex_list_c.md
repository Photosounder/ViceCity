TexList.c owns all 400 canonical texture-usage flags and all six former class
operations. CTexList and TexList.cpp are gone; the project compiles the complete
replacement as C11. No native C++ backend is needed. The public C header uses
an opaque texture pointer, and C++ declarations retain the actual original
RwTexture pointer type for both texture-returning operations.

Initialization and shutdown remain empty. SetTexture still returns null for
all arguments without reading a name or marking a slot used. The first-free
scan retains ascending order, its 400-slot bound and the minus-one exhaustion
result. LoadFileNameTexture still enters the free-slot lookup and setter.
LoadGlobalTextureList still calls FileMgr_SetDir with TEXTURES. The conversion
does not implement the original texture-loading stubs or reset the flags.

The fixture verifies four entire source/project relocations against immutable
originals, including every complete method body and the complete public header.
Native common/config, the file manager and RW texture declarations remain
unchanged. The older file-manager audit follows TexList.cpp to TexList.c and
continues comparing the whole transformed original caller; it does not discard
the caller when its old filename disappears. Earlier project snapshots also
retain their complete checks through the migration endpoint.

Both console programs expose their actual canonical arrays for observation;
the original header gains test-only public access without changing stored data
or methods. A separately compiled C11 caller enters all six operations. The
corpus tests static zero initialization, every first-free index, exhaustion,
64 sparse/hole masks, null and ordinary name pointers, extreme and invalid
slot values, repeated initialization/shutdown and changes made to flags by the
external directory owner. All 400 flags are observed after every operation.
Each run compares 15,859 states, 1,395 searches, 11,160 setter calls, 1,860
filename loads and 481 directory calls.

Twenty Clang/GCC O0/O2 comparisons cover default, plain, MASTER, unfixed and GL
profiles. Five Clang comparisons enable undefined-behavior and float-conversion
traps. Seven native optimized MSVC comparisons cover Win32 default/GL/MASTER/
vanilla and Win64 default/GL/MASTER. They all produce the same original/C result.
Native probes verify all six exact original signatures, actual RwTexture type
identity, flag scalar/array types and a standalone C11 public header.

The production C unit compiles in all five native architecture/feature profiles.
Actual Debug and Release x64 D3D9/OpenAL projects compile and link, and all fifty
project configurations keep unique object paths for the twenty-eight paired
C/C++ units. All 129 preceding source audits pass. No game or GUI test is run.

Run utils/tests/test_tex_list.ps1. Optional switches select Sanitized, Native,
Consumers, BuildProject and AuditPrevious. Immutable originals, generated test
inputs, logs and manifests are in build/tex-list-c-tests.
