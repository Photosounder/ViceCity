# Pedestrian dispatcher conversion

`src/audio/AudioPed.c` owns the complete former `ProcessPed` method as C11. The physical dispatch switch calls `AudioPed_Process` directly. The obsolete `AudioDispatchHost_Ped` trampoline and C++ owning method are removed. `AudioPedHost_OneShots` invokes the remaining pedestrian event loop through its original context.

The dispatcher retains explicit context initialization, position-copy timing, the distance-cache reset, the captured pedestrian pointer, squared-distance calculation, and event-loop ordering. It reuses the existing game position boundary and actual C geometry implementation. Owner storage and pedestrian context fields are unchanged.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_ped.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_ped_ps2.ps1
```

The audit compares the complete original method, header changes, physical switch, C++ bridge, and project registration. Runtime comparisons compile the unmodified original method against the new C implementation and a separately compiled C caller. Each comparison covers 16,384 calls over alternate owners and pedestrians, stale scratch fields, independent position axes, negative and exceptional floating values, position-query side effects, and live scratch-position changes during the camera callback. The downstream stand-in observes each initialized context field separately to avoid reading automatic padding, then mutates the context and owner state. The complete initialized owners and game position storage are hashed at callback and operation boundaries.

Results are recorded in `build/audio-ped-c-tests`: 24 Clang/GCC comparisons across two optimization levels and six feature layouts, six MSVC x86/x64 comparisons across three desktop layouts, matching desktop and PS2 sanitizer comparisons, 840 consumer compile checks, 10 configuration guard checks, and all 53 earlier migration source audits. The dispatcher regression additionally passed 20 comparisons and a sanitizer comparison exercising both actual C routing switches. Consumer compilation covers the real game headers and adapters; no full game link, gameplay test, or full PS2 game build is claimed.

`ProcessPedOneShots` remains the next owning C++ operation to convert. Its shared sound and splash histories, captured and live pedestrian queries, weapon reads, event ordering, and calls into the remaining pedestrian comment generator require preservation.
