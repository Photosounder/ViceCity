# Pedestrian event-loop conversion

`src/audio/AudioPedOneShots.c` owns the complete former `ProcessPedOneShots` loop as C11. `AudioPed.c` calls it directly. The C++ owning member and its old dispatcher trampoline are removed. Small game adapters retain the original object queries, captured weapon pointer, pedestrian-to-entity base conversion, and player lookup. The existing vehicle-model and pedestrian-comment boundaries are reused.

The original sound-counter and splash-frame statics remain shared across audio-manager instances. The conversion retains all 35 sound-event labels, weapon and surface branches, packed event parameters, live context and entity-table reads, stereo submissions, collision-report arguments, loop metadata, and calls into the remaining comment generator. The fight, pedestrian-state, and movement enums now live in `src/peds/PedStates.h`, included by both languages with their original names and values. Owner storage is unchanged.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_ped_one_shots.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_ped_one_shots_ps2.ps1
```

The source audit compares every statement in the complete original body, all private constants and sound macros, both shared histories, enum values, declarations, adapters, the direct C caller, and project registration. Runtime comparisons compile the untouched original C++ body and the separately compiled C11 implementation against the same observations. Each comparison runs 32,768 full event loops across alternating owners without resetting the shared histories. It covers all event inputs, weapon types, surfaces, movement and fight states, range boundaries, frame-counter wrap, packed parameters, signed random values, stale scratch fields, mutable context and object queries, live loop samples, stereo state, and later event-table reads.

Both implementations link the actual C math, geometry, request, and sound queue modules. The fixture observes real reflection recursion, plus arguments and timing at the separately audited collision-report and remaining comment boundaries. It normalizes game pointers while hashing the complete initialized owner, context, pedestrian, vehicle, and damage-entity records. The default fixture emits 96 distinct samples and reaches collision and reflection paths.

Inputs stay within the original defined float-to-integer conversion domains. Replacement vehicles remain valid when a mutation causes the original branch to dereference them. These fixture restrictions do not add production normalization or guards.

Verified results in `build/audio-ped-one-shots-c-tests`:

- 24 matching Clang/GCC comparisons across two optimization levels and six feature layouts
- Six matching MSVC comparisons across x86/x64 and three desktop layouts
- Matching desktop and PS2 undefined-behavior and floating-cast sanitizer comparisons
- 850 consumer compile checks and 10 configuration guard checks
- All 54 preceding migration source audits
- A fresh pedestrian-dispatcher regression after replacing its event-loop handoff

Consumer checks compile actual game declarations and adapters. No full game link, gameplay test, or full PS2 game build was performed. Pedestrian speech selection and player mood remain C++ operations for the next conversion phases.
