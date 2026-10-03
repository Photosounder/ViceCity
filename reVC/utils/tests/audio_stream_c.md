# C stream API

stream.h now defines a CStream struct with caller-owned source and buffer pointers.
The header uses standard integer types, accepts C11/C++17, and declares C linkage.
All operations use CStream_* functions with an explicit stream pointer. Open,
Setup and FlagAsToBeProcessed take explicit arguments instead of C++ defaults.
CStream_Init replaces construction and CStream_Destroy replaces destruction.
The owning sample-manager call site still allocates and frees the stream directly;
no helper allocates its owning storage. Source and buffer arrays outlive the stream.

Close every stream and join the scheduler with CStream_Terminate before destroying
its mutexes or freeing it. The worker can hold duplicate requests referencing a
stream until shutdown drains the scheduling queue. CStream_Destroy checks that
the stream is closed and releases embedded mutexes; it does not free the stream
or the caller-owned arrays.

stream.c is now a production C translation unit. It includes shared config.h,
standard C headers and the C audio headers, with no game-wide C++ header shim.
AudioStreamHost.h provides a C music-mode query and the non-Windows casepath
boundary. Diagnostics use the existing re3_debug entry with C linkage. Direct
CITA malloc/free/realloc interception remains active at the owning call sites.

CMake includes .c sources. Premake selects C/C11 and supplies /std:c11 explicitly
for Visual Studio because the bundled generator omits LanguageStandard_C.
The existing build/reVC.vcxproj and its filters reference stream.c and compile
it as C11. MSVC v143 compiles the real source with all optional decoders for
both x86 and x64. Existing decoder integer-narrowing warnings remain.

Run utils/tests/test_audio_stream_c.ps1 for fourteen direct C11 syntax checks
of the production file with clang/gcc, native/POSIX/single-thread variants,
WAV/all optional formats and 32/64-bit targets. Test configuration headers load
production defaults and explicitly select each tested variant. The external
CITA header has an old-style declaration, so strict-prototype warnings are
suppressed for this check. No stream function or header is rewritten.
Twelve C executable checks include the production file unchanged and exercise
1000 initialization/destruction cycles each. OpenAL and CITA mocks abort if
lifecycle checks unexpectedly reach hardware or allocate/free owning memory.
Two additional executables link a separately compiled C object with a C++
caller of the public header, testing native and POSIX linkage.

The worker fixture extracts the actual production functions and checks reset,
provider restart, queued-work playback status, 400 recycling cycles, 100 cutscene
cycles, duplicate work, deferred cleanup and shutdown under both threading
backends. test_audio_decoder.ps1 compares WAV/VB decode, seek, EOF and cleanup
against the saved original C++ decoder transcript. Consumer checks compile
stream.c as C and sampman_oal.cpp as C++ under D3D9/software and 32/64-bit configurations.
No GUI or real audio-device tests are performed.

This phase converts the OpenAL stream implementation to a real C source file.
The sample manager and other game subsystems still use C++. A full game build
and real-device audio playback are outside these quiet regression checks.
