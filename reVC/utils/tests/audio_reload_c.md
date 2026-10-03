# Audio reload and entity cleanup conversion

AudioReload.c owns timer reset, game-created entity cleanup and the three corresponding DMAudio facades as C11. The three old cAudioManager methods, including the empty output mode method, were removed. Music reset remains a narrow C++ callback. Script cleanup uses the existing CPools_GetAudioScriptObjectPool C getter and the actual CPool_Delete kernel directly.

Timer reset preserves the initialization gate, timer flag/value writes, active-queue restoration after clearing both order lists, partial active-record clearing, mission clear order, police stop, effects/music fades, music reset before player speech flag clearing, and optional OpenAL service. Requested sound records remain retained.

Cleanup preserves the original entity type switch. Script objects are reset before pool deletion, their entity pointer is cleared before entity removal, and the script-object manager count is reset after the scan. Unselected entity types and unused records are retained. Output mode remains a no-op for every byte argument.

Verification:

- Independent complete audits of both saved owner bodies, the two functional facades, output mode no-op semantics, music callback, and complete owner/header/facade file edits
- All 23 earlier audio source audits pass
- 16 Clang/GCC O0/O2 before/after comparisons across default, non-OpenAL, no-external and PS2 shapes
- Four native MSVC x86/x64 comparisons across default and non-OpenAL
- 512 operations per configuration cover initialization gates, both active queues, bounded channel counts, uint32 timer wrap, live global facades, alternating owners, mixed and unknown entity types, script objects with valid or null pointers, and repeated cleanup
- Actual C entity removal, sound clearing, mission clearing and mission position modules are linked; actual CPool_Delete is used. The unchanged production AudioScriptObject_Reset body is extracted into a standalone C test unit to exclude unrelated script save/load dependencies
- Complete zero-padded owner storage is observed with normalized pointer identities, together with real pooled script object bytes, generation/free flags and allocation cursor, at every backend/music/pool callback and after operations
- Callback-driven timer mutation detects ordering changes and premature state copying
- 440 unique consumer compilation checks across the existing architecture, renderer, threading and decoder matrix; actual native game callers, facade consumers and new C owner compile on x86/x64

Full game linking and playback were not tested. Script save/load behavior and music-manager implementation were not changed.
