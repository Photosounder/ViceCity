# C OpenAL channels

channel.h exposes a plain CChannel struct, CChannel_* operations, and a global
CChannel_channelsThatNeedService counter with C linkage. channel.c compiles
as C without common.h or the C++ sample-manager header. AudioSampleConfig.h
shares the original sample rate, volume, format and ped-block constants with
sampman.h. The existing Visual Studio project and filters compile channel.c
as C11; the CMake and Premake C-file rules also include it.

CChannel_InitializeState replaces the former constructor. The sample manager
initializes its embedded array once on first initialization, preserving state
reuse across provider changes and later audio initialization. Init binds a
source ID and takes an explicit 2D flag. Term stops playback and releases source
attachments; DestroyChannels releases shared OpenAL objects. Callers own the
channel storage and sample data. These helpers do not allocate owning memory.

The old IsFXSupported query and game assertion handler now have C linkage.
CHANNEL_ASSERT retains the game's debug handler and MASTER expression evaluation
instead of changing assertion behavior through the standard C assert macro.
The existing finite-loop counter, initial signed offset, integer PCM expansion,
2D flag retention during source reuse, and reverb mix retention are preserved.

Run utils/tests/test_audio_channel.ps1 for eight clang/gcc C11/C++17 O0/O2 cases.
The current fixture includes production channel.c unchanged. Each run compares
exact device-operation and uploaded-PCM traces with the saved original C++ source
and header in build/audio-channel-c-tests. The baseline substitutes the old
common/sampman includes with their original primitive aliases and constants.
Its original channel methods and calculations are retained.

Each run checks 400 cycles on both mono and 2D channels, finite/infinite loop
transitions, stopped-source recovery, maximum 79000-byte mono and 158000-byte
stereo uploads, source parameters, reverb update gates, reset, source reuse,
effects-disabled creation, duplicate destruction and absent sources. The trace
contains 32572 mock calls and ends with a zero service count. A separate C object
links with a C++ caller of the public header and exercises its runtime API.

Forty consumer syntax checks cover stream.c, sampman_oal.cpp, oal_utils.c and
channel.c under D3D9/software, 32/64-bit targets, threaded/single-thread paths and
WAV/all optional decoder selections. MSVC v143 compiles channel.c as C11 for
x86 and x64; existing size_t-to-ALsizei conversions warn on x64. No window or
real audio device is opened, and no full game build is claimed.

The OpenAL device list now uses C in aldlist.c; see audio_device.md. The larger
sample-manager class and other systems remain part of the C++ conversion.
