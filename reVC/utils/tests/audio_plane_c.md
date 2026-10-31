# Plane and Jumbo audio conversion

AudioPlane.c owns plane routing, all six Jumbo flight stages, five sound setup operations and taxi-frequency calculation as C11. All former members now take explicit manager and parameter or opaque vehicle pointers. The owning cAudioManager record is unchanged. Visual Studio registers the new C11 compilation unit; CMake and premake already discover C sources.

The shared Jumbo volume offset is file-local C storage. Its update still reads the global AudioManager frame counter and random table even when a different manager processes a plane. Taxi volume retains its integer percentage subtraction; engine volume retains its separate integer division by 100. No arithmetic cleanup or behavior change is included.

Typed C++ adapters expose captured plane speed, live path position through its plane ID, and the current takeoff and landing thresholds. Flight routing, range gates, layer blending, frequency arithmetic, queue field writes, callback ordering and short-circuit behavior live in C. The original Min expansion retains its selected repeated speed read through a comparison barrier. Plane routing uses the existing audio-table indices and forwards Cessna to its separately audited C generator.

Setup helpers still return success when the distance gate passes even when attenuation produces zero volume. Stereo rumble retains two submissions, counters 5 and 6, opposite pans, and a 200 Hz increment applied to live scratch frequency after the first submission. Existing conditional emitting-volume, loop-offset, reverb and reflection fields are preserved.

Verification:

- Complete original bodies, global offset update, constants, adapters, macros, declarations and caller changes audited against before-conversion snapshots
- Visual Studio project and filter changes checked against their original snapshots
- All 49 earlier audio source audits pass
- 790 unique compiler checks cover 79 production consumers across ten x64/x86, D3D9/software, threaded/single and WAV/all-decoder combinations, including actual Plane.cpp
- Ten independent preprocessor guards check the intended thread and decoder combinations
- 24 Clang/GCC O0/O2 before/after comparisons cover default, vanilla, no reverb, no external audio, PS2 and disabled audio time scaling
- Six native MSVC x86/x64 comparisons cover default, no reverb and disabled time scaling; game audio and plane consumers compile
- Each comparison exercises 32,768 calls across all 14 original entry points, alternating global/local owners, retained offset history, six flight states, exact path/speed/range boundaries, cached distance and mutations between backend queries and submissions
- Every sound layer and Cessna forwarding path is required; desktop fixtures also require live loop-offset queries
- Fixtures compare complete initialized owner, parameter, vehicle and path records at callbacks and operation boundaries, with normalized vehicle pointers
- Original and converted bodies both use actual C math, geometry, request insertion and ordered sound queues; a wrapper observes real request submission before applying fixture mutations
- Undefined-behavior and float-cast-overflow sanitizer comparisons pass for desktop and isolated PS2 branches

Run utils/tests/test_audio_plane.ps1 for the source audit, behavior comparisons and desktop sanitizer check. Native compiler, consumer matrix, mode guard and supplemental PS2 sanitizer scripts and results are in build/audio-plane-c-tests. Full game linking and in-game playback remain unverified. The isolated PS2 branches do not establish a complete PS2 game build with the Windows toolchain.

The main ProcessVehicle dispatcher is the next remaining vehicle-audio operation to move into C. Its existing generators already use C APIs; the dispatcher still owns C++ game-field reads, transmission selection, velocity calculation and vehicle-type routing.
