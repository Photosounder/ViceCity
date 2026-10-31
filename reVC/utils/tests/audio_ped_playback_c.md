# Pedestrian comment playback conversion

`src/audio/AudioPedPlayback.c` owns the complete `PedComments_Process` playback body and the former `cAudioManager::DebugPlayPedComment` producer. The debug menu calls `AudioPedPlayback_Debug` with an explicit owner. One typed C++ adapter copies the original player-zero position; the loading backend, timer, and slow-motion boundaries use existing C declarations.

The conversion retains the process-wide ten-sample history, pause gate, repeated live priority-list reads, captured sample identifier, mission versus pedestrian loading, playback scratch fields, police comment distance, optional player-comment 2D mode, frequency calculation, history update before submission, timeout marking, queue advancement, and optional PC delay behavior. Playback still submits through the global `AudioManager`, including when processing another owner's comment queue. Debug insertion uses its explicit owner. No stored fields or object layouts changed.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_ped_playback.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_ped_playback_ps2.ps1
```

The source audit compares every statement in both complete original bodies, their typed substitutions, the position adapter, debug-menu call, declarations, and project registration. Runtime comparisons use unchanged original C++ bodies and separately compiled C11 operations, with the real C queue insertion/advancement and random-displacement implementation on both sides. The request submission observer checks global ownership, callback ordering, and complete scratch sound fields; it does not rerun the request/reflection implementation itself.

Each comparison performs 32,768 calls with alternating owners and queues while retaining shared history. It covers history wrap and eviction, repeats, pause/empty queues, mission and pedestrian loading outcomes, police/player sample boundaries, callback changes to live queues and scratch samples, pending-comment carryover, automatic debug records, slow motion, and delay expiry with unsigned timer wrap. Complete initialized owners are hashed at callbacks and after each operation. Only unspecified trailing padding in automatic comment records is normalized.

The original optional PC configuration without `FIX_BUGS` reads an uninitialized `sampleIndex` on non-duplicate active playback paths. This migration preserves that pre-existing issue. Its runtime fixture exercises defined pause, empty-queue, `NO_SAMPLE` duplicate rejection, debug insertion, carryover, and delay-expiry paths; it does not claim equivalence for undefined playback. Full statement audits retain that branch verbatim apart from C types and existing timer declarations.

Verified results in `build/audio-ped-playback-c-tests`:

- 28 matching Clang/GCC comparisons across two optimization levels and seven feature layouts
- Six matching native MSVC comparisons across x86/x64 and three desktop layouts
- Matching desktop and PS2 sanitizer comparisons
- 880 actual consumer compile checks and 10 configuration guard checks
- All 57 preceding migration source audits
- 16 matching queue regressions after following the migrated playback and debug bodies

No full game link, gameplay test, or full PS2 game build was performed. The next self-contained audio operation to inspect is `ProcessExplosions` in `AudioLogic.cpp`.
