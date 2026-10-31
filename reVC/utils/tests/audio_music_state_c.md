# Independent music operations in C

`MusicManager.h` now exposes `tStreamedSample` and all stored `cMusicManager` fields as fixed-width C records. Remaining member declarations and game-type forwards are guarded for C++. The global music owner and announcement flag have C linkage. No fields were reordered or removed.

`src/audio/MusicState.c` owns twelve complete operations: constructor field initialization, three state readbacks, deferred timer reset, script radio selection, frontend track selection/stop, announcement selection, listen-time array access, favourite-station selection and interruption detection. Converted methods are removed from the header and implementation. All global callers and remaining self calls enter the C functions directly. There are no new game adapters or C++ operation trampolines.

The existing C++ constructor now calls `MusicState_InitState` at its original compiler startup point. It remains a C++ feature, along with the remaining gameplay, service and provider-dependent music methods. The separate audio owner's startup registration problem remains open. This phase does not change that registration.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_music_state.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_music_state_sanitized.ps1
```

The full source audit compares every moved body, all original inline readbacks, each stored field and track record, the constructor callback, all four changed caller files, and project entries. Earlier migration audits explicitly follow the same music conversion endpoint.

Each runtime comparison performs 36,864 operations across 1,024 seeds, two owners and three passes. Both use the complete original operation or the separately compiled actual C implementation and a caller compiled against the production C header. Checks hash both complete initialized records after every operation. The returned listen-time pointer is checked for its original alias and used to mutate the owner through that alias. Coverage includes nonzero byte flags, initialized/disabled/announcement gates, frontend modes and bounds, MP3 mapping, invalid script station bounds, signed position sentinels and wrapping arithmetic with valid nonzero divisors, repeated timer/initialization calls, favourite-station ties and quiet NaNs/infinities, and both interruption tracks.

Verified results under `build/audio-music-state-c-tests`:

- 16 Clang/GCC original-versus-C comparisons across two optimization levels and four feature configurations
- Four native MSVC x86/x64 comparisons with `/GL`, `/LTCG`, and `/OPT:REF`
- Two undefined-behavior/float-cast-overflow sanitizer comparisons in trap mode
- 1,080 production-consumer compilation checks and 10 thread/decoder guards
- Native C/original C++/current C++ ABI arrays match all 101 owner metadata entries on x86 and x64: size 14,796 bytes, alignment 4
- All 69 preceding migration source audits

The original stock-size validation remains conditional on `CHECK_STRUCT_SIZES`. This project's expanded streamed-sound enumeration already makes the original owner larger than its stock `0x95C` constant; an unconditional C assertion would introduce a new failure. The native ABI comparisons verify the actual expanded layout instead.

No complete game link or gameplay test was performed. The next independent operations are music stream termination and cutscene playback, whose existing sample-provider calls already have C APIs.
