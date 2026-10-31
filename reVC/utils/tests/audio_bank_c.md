# Remaining audio-owner helper conversion

`src/audio/AudioBank.c` owns the complete former `LoadBankIfNecessary` operation as C11 under its original `GTA_PS2` guard. Its C API accepts the same byte-sized bank argument. The loaded-bank query still controls the load call, its signed-byte result still uses the original zero/nonzero gate, and the load return value remains ignored. The operation needs no owner pointer because the original member never accessed owner state.

The unused `cAudioManager::Sqrt` inline member is removed. A project-wide source search found no references to either remaining helper beyond their declarations/definition. `AudioManager.h` now retains only constructor and destructor declarations in its C++ method block; its stored fields and conditional layout are unchanged. The Visual Studio project registers the C11 source, and existing CMake/Premake globs discover it.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_bank.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_bank_desktop.ps1
```

The source audit compares the complete original body, every owner field and the full owner/header/project changes. Runtime comparisons compile either the unchanged original C++ member or the actual separate C11 operation. Both use identical provider observers with the exact signed-query/byte-bank signatures.

Each comparison performs 12,288 groups across 4,096 seeded inputs. Two passes exercise live mutations and repetition; a fresh pass retains the seeded query boundary. Inputs cover all 256 byte-valued banks, unsigned argument wrapping, zero/positive/negative loaded-bank results, provider success/failure and both owners. Callbacks alter loaded state after its captured read, change the next raw input, toggle initialization, change provider results and mutate sample/owner fields. Complete owner and provider storage is initialized with zero padding and hashed at callbacks and after groups. The disabled desktop feature requires zero provider callbacks.

Verified results in `build/audio-bank-c-tests`:

- 16 matching Clang/GCC comparisons across two optimization levels and four feature layouts
- Four matching native MSVC comparisons across x86/x64 and enabled/disabled layouts
- Matching enabled PS2 and disabled desktop sanitizer comparisons
- 1,060 actual consumer compile checks and 10 configuration guards
- All 67 preceding migration source audits

The constructor and destructor are the next owner features to remove. The native MSVC object places owner initialization in `.CRT$XCU` and registers its destructor through `atexit`; `build/audio-bank-c-tests/owner-startup-msvc64.txt` records the disassembly. The C replacement must preserve the five pre-main random draws, exit registration timing, conditional audio termination and unconditional script-state reset. Removing these methods requires an explicit C startup/exit mechanism, not postponing initialization until the first audio call.

No full game link, gameplay test or complete PS2 game build was performed. The bank-helper fixtures use existing provider APIs and need no fixture-only legacy sample or bank definitions.
