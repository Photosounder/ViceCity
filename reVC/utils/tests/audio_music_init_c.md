# Music initialization and starting-track selection in C

`src/audio/MusicInit.c` owns the complete former music-manager `Initialise` and `SetStartingTrackPositions` operations, the writable `NewGameRadioTimers[10]` storage and the public starting-position facade. Converted C++ member declarations and definitions are removed. Audio lifecycle initialization calls `MusicInit_Initialise(&MusicManager)` directly; the obsolete C++ music-initialization trampoline and declaration are removed.

The complete clock/random calculation and every partial state write are retained. Standard `time`/`localtime` calls run directly in C, including the error fallback and mutation of each originally zero broken-down-time field. The original signed arithmetic is unchanged. Every logical track still calls the existing byte-parameter file-length API, stores the signed result in unsigned track storage, selects its original start position, then queries the pause-mode timer. Indices above 255 retain the original byte narrowing. New-game table selection, live random-table reads, zeroing of later tracks and timer reads stay at their original points.

One typed C++ adapter remains: `MusicInitHost_PauseTime` reads `CTimer::GetTimeInMillisecondsPauseMode`. No initializer, loop or time/random operation is implemented by that adapter. The retune counters are exposed with C linkage while their existing C++ storage remains in place. No music or audio record fields were changed. C++ startup wrappers still remain pending resolution of initialization ordering.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_music_init.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_music_init_sanitized.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_lifecycle.ps1
```

The source audit checks both complete original operations, original writable table values/storage removal, the public facade, the exact pause-time adapter, direct lifecycle routing and all header/caller/project edits. Earlier source audits explicitly follow this conversion.

Each comparison runs 192 groups across 32 seeds, two passes, two music owners and three operation/facade paths. It observes 313,572 callbacks. Complete zero-padded music/audio/provider records, broken-down-time storage, writable timer table, counters and arguments are hashed at each callback and after groups. The clock fixture uses the exact standard-library parameter/result types, replacing calls only in test builds after including the real time header. The pause timer observer is shared by the original inline timer query and the new C boundary.

Inputs include closed/already-initialized gates, raw nonzero byte results, clock failure, every individual zero-field substitution, all-zero and mixed masks, ordinary calendar values, new-game flags, random-table mutations between track reads, original byte narrowing through the full expanded track count, positive and negative captured file lengths with nonzero unsigned divisors, pause-timer wrapping and live callback mutations of flags/counters/tables. They verify that callbacks can change the final initialized return byte and that initializer resets occur after starting-track callbacks.

Verified results in `build/audio-music-init-c-tests`:

- 16 Clang/GCC comparisons across two optimization levels and four feature configurations
- Six native MSVC x86/x64 comparisons with `/GL`, `/LTCG` and `/OPT:REF`
- Two undefined-behavior/float-cast-overflow sanitizer comparisons in trap mode, restricted to defined arithmetic inputs
- 16 audio-lifecycle regression comparisons
- 1,100 production-consumer compile checks and 10 thread/decoder guards
- All 71 preceding migration source audits

## Existing arithmetic limitation

The unchanged original calendar multiplication overflows signed integers for ordinary calendar inputs. The extracted original expression with a representative calendar value has a mathematical product of 1,017,339,828,353,630,208 and traps under the local Clang undefined-behavior sanitizer (`original-overflow.cpp` / `original-overflow.log`, exit -1073741795). The diagnostic executable suppresses Windows fault dialogs. The random-position multiplication also retains its original signed arithmetic.

The default comparisons include these existing overflow paths and compare observed results at the tested compiler/optimization settings. They do not establish portable defined behavior for overflowing arithmetic. Sanitizer comparisons use bounded calendar fields and random values, so all arithmetic is defined while exercising the same full loops, callbacks, conversions and reset operations. This phase introduces no overflow correction; that is separate behavior work.

No full game link or gameplay test was performed. Remaining music work includes reload/statistics/track-position operations and gameplay/service routing.
