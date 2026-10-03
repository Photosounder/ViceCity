# C OpenAL device list

aldlist.h defines plain ALDEVICEINFO records, a fixed ALDeviceList struct and
ALDeviceList_* functions with C linkage. aldlist.c compiles as C without the
old constructors, destructors, reference binding or overloaded source getter.
The private source probe has a distinct C name. Device-name allocation occurs
inside ALDeviceList_Init with CITA tracking at the owning call site.

The sample manager retains one cached list. Provider IDs borrow its owned name
strings, so termination or provider switching does not destroy the list.
Explicit initialization replaces the function-static C++ constructor, and an
atexit callback replaces its process-exit destructor. Destroy releases names
in the old reverse array-destruction order and makes repeated cleanup safe.
Callers must destroy a initialized list before initializing it again.

Two unsafe old edges are repaired: enumeration retains at most 64 accepted
records, and failed bulk source deletion retries only successfully generated
handles. Extension queries reject negative and out-of-capacity indexes before
reading a record. Normal enumeration order, duplicate actual device names,
version and extension filtering, default-device indexing and iterator sentinel
behavior match the original implementation.

Run utils/tests/test_audio_device.ps1 for eight clang/gcc C11/C++17 O0/O2 cases.
Each compares normal device-operation, ownership and query traces with the
saved original source/header in build/audio-device-c-tests. The baseline keeps
its original constructors, methods and destructors and redirects allocation to
quiet tracking functions. The new fixture includes aldlist.c unchanged and
supplies mock CITA and OpenAL API implementations. No device is opened.

Each run covers 101 enumerations, including empty lists, missing enumeration
support, rejected devices and contexts, null/empty/duplicate actual names,
version and extension combinations, all eight combinations of minimum/maximum
version and extension filters, and source capacities of 0, 1, 7, 255 and 256.
The normal comparison includes exactly 64 records. Separate current-only checks
cover 72 accepted device candidates with guarded storage, invalid indexes,
failed bulk deletion, duplicate cleanup and zero outstanding allocations,
contexts, devices and sources.

The four C++ fixture runs additionally extract the actual cached-list lifetime
and add_providers functions from sampman_oal.cpp. They verify names survive
repeated provider setup and changed driver enumeration strings, initialization
and exit registration occur once, and exit cleanup frees names exactly once.
A separate C object links with a C++ caller of the public API.

Fifty consumer syntax checks cover stream.c, sampman_oal.cpp, oal_utils.c,
channel.c and aldlist.c under both renderers, 32/64-bit targets,
threaded/single-thread variants and decoder selections. MSVC v143 compiles
aldlist.c as C11 for x86 and x64. The existing Visual Studio project and filters
reference the C file; the CMake and Premake C-file rules include it as well.
A full game build and real-device enumeration are outside these quiet checks.

The larger sample-manager class and other game systems still use C++ and remain
part of the ongoing whole-project conversion.
