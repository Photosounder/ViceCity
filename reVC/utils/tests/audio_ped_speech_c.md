# Speech setup and player mood conversion

`src/audio/AudioPedSpeech.c` owns the complete former `SetPedTalkingStatus`, `SetPlayersMood`, `ProcessPlayerMood`, and `SetupPedComments` operations as C11. It also owns the public `DMAudio_SetPedTalkingStatus` and `DMAudio_SetPlayersMood` wrappers. Environment service and both pedestrian/vehicle event loops call the C operations directly. Their obsolete mood/comment trampolines are removed.

The conversion preserves null and initialization gates, boolean talking permission, timer addition and wrap, mood expiration, repeated wanted-level and mission-time queries, mission-time correction, captured pedestrian speech selection, generic speech branches, both police-boat sample-count variants, visibility arguments and flags, attenuation, live comment coordinates and entity index, conditional emitting volume, and real C queue insertion. Existing game timer/player/wanted boundaries are reused. Owner and comment layouts are unchanged.

The private C distance constants retain the original values. Their C++ counterparts remain for the unmigrated comment-playback code that still uses them. `GetPedCommentSfx` and character-specific speech selection remain the next C++ operations to convert.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_ped_speech.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_ped_speech_ps2.ps1
```

The audit checks complete original bodies, public wrappers, adapters, direct callers, declarations, constants, project registration, and unchanged owner fields. Runtime comparisons compile the original C++ bodies and the separately compiled C11 module against the same observations. Each comparison covers 32,768 operations, alternate and global owners, null pedestrians/players, all mood inputs and wanted thresholds, timer equality and wrap, mission-time boundaries, mutable repeated game queries, selector and visibility callbacks, range boundaries, signed random values, and comment queue filling/replacement, with actual insertion required from all four generic speech branches. Both sides use the actual C math and comment queue modules.

The fixture hashes all initialized owner and game records with normalized pointers. Only unspecified trailing padding in automatic `tPedComment` records is excluded; every defined comment field remains part of the comparison. Inputs avoid original undefined floating-to-byte conversion domains.

Verified results in `build/audio-ped-speech-c-tests` include 24 matching Clang/GCC comparisons across two optimization levels and six feature layouts, six matching native MSVC x86/x64 comparisons, matching desktop and PS2 sanitizer comparisons, 860 consumer compile checks, 10 configuration guard checks, and all 55 preceding migration source audits. Environment (20 comparisons), vehicle-event (32), and pedestrian-event (24) regressions also passed, each with a sanitizer comparison, after trampoline removal.

Consumer checks compile actual game declarations and game adapters. No full game link, gameplay test, or full PS2 game build was performed.
