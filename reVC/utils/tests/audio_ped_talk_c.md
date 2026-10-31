# Complete pedestrian speech selection conversion

`src/audio/AudioPedTalk.c` owns the complete former `GetPedCommentSfx` router, `GetPhrase` reference helper, and all 96 character speech handlers as C11. Speech setup calls `AudioPedTalk_Select` directly. Their C++ owning declarations and definitions, obsolete selector trampoline, and old cooldown macro are removed.

The conversion preserves the falling/damage/get-up gates, repeated timer and model reads, player and model routing, special-character model recheck, generic fallbacks, player mood and shut-up behavior, signed random-table inputs with the original unsigned modulo arithmetic, per-pedestrian previous-comment writes, repeat avoidance and wrap, and all conditional `FIX_BUGS` branches. Each of the ten cooldown expansions retains its own original shared byte-sized static. No owner fields or game-object layouts were added.

Four small game adapters read the get-up timer/model, resolve the writable previous-comment field, and query live player state. Existing pedestrian state/player, game timer, and wanted-level boundaries are reused. The phrase helper now takes explicit writable pointers, including the original behavior when its two reference arguments alias.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_ped_talk.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_ped_talk_ps2.ps1
```

The source audit compares every statement in all 98 original operations, the complete cooldown macro and all ten expansion sites, declarations, adapters, the direct C caller, project registration, and unchanged owner storage. Runtime comparisons compile the original C++ bodies and the separately compiled C11 implementations against identical observations. Each comparison runs 65,536 calls across all handlers and their sound-event combinations, alternating owners and pedestrians while retaining shared cooldowns and per-pedestrian phrase histories. The fixture includes successful/rejected selections, model/state gates, mutable repeated reads, wanted/player queries, mood/shut-up state, signed random values, timer boundaries, unsigned sample/offset wrap, one-sample ranges, and aliased phrase pointers. Zero offsets, which make the original modulo undefined, are excluded.

The observer hashes complete initialized owner, pedestrian, wanted, and phrase records with normalized pointers. Both original and converted logic produce the same return values, history writes, and game-query order.

Verified results in `build/audio-ped-talk-c-tests`:

- 24 matching Clang/GCC comparisons across two optimization levels and six feature layouts
- Six matching native MSVC comparisons across x86/x64 and three desktop layouts
- Matching desktop and PS2 sanitizer comparisons
- 870 consumer compile checks and 10 configuration guard checks
- All 56 preceding migration source audits
- A fresh speech-setup regression after replacing its selector trampoline

Consumer checks compile actual game headers and adapters. No full game link, gameplay test, or full PS2 game build was performed. Comment playback and the debug comment producer remain the next speech operations to convert.
