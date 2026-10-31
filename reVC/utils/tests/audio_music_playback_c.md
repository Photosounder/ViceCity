# Music stream termination and cutscene playback in C

`src/audio/MusicPlayback.c` owns the complete former music-manager operations `Terminate`, `PreloadCutSceneMusic`, `PlayPreloadedCutSceneMusic`, and `StopCutSceneMusic`, together with all three public cutscene facades. Their C++ member declarations/definitions and facade definitions are removed. The C audio lifecycle calls `MusicPlayback_Terminate(&MusicManager)` directly; the former C++ `AudioLifecycleHost_MusicTerminate` trampoline and prototype are removed.

The conversion adds no game adapters. Music readback, sample-provider calls and police reset already have C APIs. It preserves the initialized/disabled/mode gates, captured byte query result, original track bounds and captured track parameter, police reset before polling, repeated stop/query loop, exact preload/volume/pan/effect/stream arguments, and post-callback track/initialized writes. A zero query result during termination retains the playing track except for any provider callback's own live mutation.

All music and audio owner fields are unchanged. The separate C++ startup wrappers remain, pending resolution of startup registration ordering; this conversion does not alter their callbacks.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_music_playback.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_music_playback_sanitized.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_lifecycle.ps1
```

The source audit checks every complete moved body, all three original public facades, unchanged music fields, direct lifecycle routing and full caller/header/project edits. Earlier source audits explicitly follow the music playback conversion.

Each runtime comparison performs 21,504 groups across 1,024 seeds, three passes and seven operation/facade paths, using two music owners. It compiles the unchanged preceding C++ operations/facades or the actual separate C11 implementation and a caller compiled against production C declarations. Both share the actual C music readbacks and police reset/initialization. Provider callbacks observe complete owner, sample-provider and police-global storage and then mutate live flags, mode, track state, captured-input storage, provider volume and audio frame counters. Delayed stream-stop acknowledgements require up to four stop calls and five queries; query results include zero, one and 255. Inputs exercise closed gates, active cutscene playback, boundary/out-of-range tracks, both owners, repeated operations and ignored live gate changes after entering an operation. PS2 configurations cover police reverb callbacks enabled and disabled.

The isolated audio-lifecycle regression still observes the original music callback token at the new C operation entry and verifies the actual C music-owner pointer. A separate C fixture supplies the correct owner symbol; the legacy fake music observer has a separate name. Complete music cleanup behavior is covered by the playback comparisons, while this regression checks its orchestration point before entity clearing and backend termination.

Verified results under `build/audio-music-playback-c-tests`:

- 20 Clang/GCC comparisons across two optimization levels and five feature configurations
- Six native MSVC x86/x64 comparisons with `/GL`, `/LTCG` and `/OPT:REF`
- Two undefined-behavior/float-cast-overflow sanitizer comparisons in trap mode
- 16 audio-lifecycle regression comparisons
- 1,090 production-consumer compile checks and 10 thread/decoder guards
- All 70 preceding migration source audits

No full game link or gameplay test was performed. Remaining music-manager operations include initialization, starting-track selection, gameplay/service routing and timer/statistics work.
