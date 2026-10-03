# Vehicle rain sound generator conversion

AudioVehicleRain.c owns the former ProcessRainOnVehicle member as C11. ProcessVehicle passes its manager and existing cVehicleParams record explicitly. The two rain constants move from the C++ game enum to the C unit. Weather/rain exclusion getters and a counter-address adapter remain in AudioLogic.cpp; counter arithmetic and sound request generation run in C. The adapters expose the original vehicle bytes without changing CVehicle or cVehicleParams storage.

The conversion preserves distance-before-weather eligibility, strict rain and distance thresholds, short-circuit camera/player rain exclusions, vehicle capture after those checks, rainfall reread for emitting volume, byte counter postincrements and wraparound, audio counter reset, the original sample-counter reset to 68 after values above 4, distance caching for every nonzero flag byte, sample variation, frequency and request fields. No initialization or pause gates are added. Visual Studio registers the C unit; CMake and premake already discover C sources.

Verification:

- Complete generator body, adapters, private constants, conditional sound macros, header and internal caller edits audited against saved original sources
- All 36 current and earlier audio source audits pass
- 20 Clang/GCC O0/O2 before/after comparisons across default, vanilla, no reverb, no external audio and PS2 shapes
- Four native MSVC comparisons across x86/x64 and default/no-reverb configurations, plus actual game audio and physical caller compilation
- 19,200 operations per comparison exercise both owners, actual C callers, repeated calls, weather/distance thresholds, NaN or infinite rejected distance, rain rejection including NaN, camera/player exclusions, cached distance flag bytes, retained sample fields, random values and empty/partial/full request queues
- Every audio counter byte is tested against all sample counter boundary values, and every sample counter byte is tested against normal and wrapping audio counter states
- Actual C audio math, request insertion and ordered sound queue units are linked in both variants
- Complete owner and parameter bytes are observed with normalized vehicle identities, plus both vehicles' counter bytes and weather/request query order
- Camera checks mutate rainfall and vehicle selection; the second rainfall read can change parameter vehicle selection after the original vehicle was captured; request callbacks mutate live vehicle counters
- 570 unique consumer compilation checks pass across architecture, renderer, threading and decoder configurations
- An additional Clang O1 comparison with undefined-behavior and floating-conversion checks in trap mode passes

Full game linking and playback were not tested. Weather and room queries are controlled in the differential fixture, while actual C math and sound queue insertion run. Test rainfall values and accepted distances remain within the original defined conversion ranges.
