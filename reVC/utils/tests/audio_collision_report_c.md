# Collision reporting conversion

AudioCollisionReport.c owns collision reporting and the existing DMAudio_ReportCollision public facade as C11. The C++ ReportCollision member is removed. The internal ped impact caller passes its owning manager explicitly. Physical and vehicle callers keep the existing public facade. Two small C-callable game adapters retain IsBuilding and GetPosition access in AudioCollision.cpp. Visual Studio registers the C translation unit; CMake and premake already discover C sources.

The report retains the original early rejection order, building short-circuit order and single position query on building paths, separate float addition and scaling for the midpoint, weighted camera distance, strict squared-distance boundary and request insertion order. It preserves queued base volumes and owner storage. No allocations are added.

Verification:

- Complete report body, both game adapters, owner/header edits, internal caller and public facade audited against the saved original sources
- All 34 current and earlier audio source audits pass
- 20 Clang/GCC O0/O2 before/after comparisons across default, vanilla, no reverb, no external audio and PS2 configurations
- Four native MSVC comparisons across x86/x64 and default/no-reverb configurations, with actual game audio and physical caller compilation
- 8,192 reports per comparison exercise both owners and actual C callers, initialization/pause/entity gates, intensity thresholds, building shortcuts, aliased entities, empty/partial/full queues and replacement, all surface bytes, signed zero, distance boundaries, overflow, infinities and NaNs
- Actual C collision queue and geometry units are linked in both implementations
- Every owner byte is observed after each report, with normalized entity pointers; game query order is hashed and callbacks mutate previously checked gates and entity positions
- 550 unique consumer compilation checks pass across x86/x64, D3D9/software, threading and decoder configurations, including physical and vehicle callers
- All 24 audio-effects regression comparisons retain their prior digests
- An additional Clang O1 comparison with undefined-behavior and float-conversion checks in trap mode passes

Full game linking and playback were not tested. Game entity queries are controlled in the differential fixture. One-shot and looping collision sound generation remain in C++ and are the next conversion candidates.
