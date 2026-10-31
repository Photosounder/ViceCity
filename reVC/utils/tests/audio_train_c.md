# Optional train audio conversion

`src/audio/AudioTrain.c` owns the complete former `ProcessTrainNoise` generator as C11. The two remaining C++ adapters read speed and wagon position from the captured game train. The C function retains the original range and motion gates, repeated speed reads, far and near layers, signed frequency remainder, loop metadata, request ordering, and live distance recheck after the far request. No owner fields or callers were added.

`GTA_TRAIN` is disabled in this checkout, and its sample enum does not define `SFX_TRAIN_FAR` or `SFX_TRAIN_NEAR`. These limitations already applied to the original generator. The isolated fixture supplies synthetic sample identifiers solely to compare both implementations; it does not establish real audio asset mappings or enable train gameplay.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_train.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_train_ps2.ps1
```

The source audit checks the full saved original body, macros, constants, declarations, adapters, and project registration. Runtime comparisons insert that original body directly into the C++ fixture and link the new implementation through a separately compiled C caller. Both link the actual C math, geometry, request, and sound queue implementations. Each comparison exercises 16,384 calls, alternating owners, range and motion boundaries, null vehicles on rejected paths, speed caps, wagon gates, signed entity indices, cached and calculated distances, loop callbacks, and mutations between requests. Complete initialized state and callback order are hashed with normalized pointers. Clang and GCC also observe repeated field reads; MSVC uses pure floating field reads because its treatment of instrumented conversion operators differs.

Active negative or NaN speeds and attenuation inputs that make the original floating-to-byte cast undefined are excluded. Production behavior is not changed to normalize those inputs.

Verified results in `build/audio-train-c-tests`:

- 24 Clang/GCC comparisons across two optimization levels and six feature layouts
- Six native MSVC comparisons across x86/x64 and three desktop layouts
- Matching desktop and PS2 undefined-behavior and floating-cast sanitizer runs
- 830 normal consumer compile checks, 30 guarded train consumer checks, and 10 configuration guard checks
- All 52 preceding migration source audits

The guarded C compile checks use test-only sample identifiers; the C++ adapters compile against the actual game train declaration. No full game link, gameplay test, or full PS2 game build was performed.

The next conversion is the small `ProcessPed` dispatcher, followed by `ProcessPedOneShots` and the remaining pedestrian comment logic.
