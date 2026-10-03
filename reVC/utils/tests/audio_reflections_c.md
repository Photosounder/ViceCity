# Reflection probe update conversion

AudioReflections.c owns the former cAudioManager::UpdateReflections scheduler and distance arithmetic in C11. The service caller passes the same live owner. Existing camera-position access is reused, and two narrow C++ world adapters preserve the line/vertical queries and copy successful collision results.

Both version branches remain verbatim after coordinate and callback substitutions: early builds use five 50-unit probes across eight frame phases, while current builds use four horizontal 100-unit probes and four offset ceiling probes. Frame-counter additions retain uint32 wrap. Reflection positions are written before world callbacks; distances are changed only afterward. Failed probes retain the version-specific fallback. Horizontal hit distances use the original unweighted, unclamped vector magnitude. Ceiling distances retain signed hitZ-cameraZ subtraction, including negative values.

Verification:

- Independent complete statement audit of both schedules against the saved member, and of the scalar distance against Vector.h subtraction/magnitude and global sqrtf
- Complete owner/header/service and world adapter audits; all 22 previous audio source audits pass
- 20 Clang/GCC O0/O2 before/after comparisons across current, early, no-external, PS2 and no-reflection shapes
- Four native MSVC x86/x64 before/after comparisons covering both version schedules
- 1,024 cases per enabled configuration cover each frame phase, uint32 overflow, alternating owners, hit/miss behavior, zero/positive/negative ceiling heights, exact query flags, null polygon output, callback-visible endpoint and distance writes, and world callbacks that mutate camera/frame state
- 430 unique consumer compilation checks across the existing architecture, renderer, threading and decoder matrix
- Actual game callers, world adapters and C owner compile with native MSVC x86/x64; AudioManager.cpp also compiles for the early version
- C++ collision point and entity output handling remains at the world boundary; no game object layout is assumed by C

Full game linking and playback were not tested. World collision implementations and renderer/camera construction were not modified.
