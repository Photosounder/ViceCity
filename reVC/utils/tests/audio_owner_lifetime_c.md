# Audio owner cleanup in C

`src/audio/AudioOwner.c` now owns the complete destruction operation as C11. `AudioOwner_Destroy` conditionally terminates initialized audio, then always resets the script-object count. It preserves retained script indices and all other owner fields. The existing C++ destructor delegates to this function.

The constructor, global owner definition, and compiler-generated startup/exit registration remain in `AudioManager.cpp`. Removing them is unfinished. A direct `.CRT$XCU` function-pointer replacement changed startup order under native MSVC `/GL` and `/LTCG`: the five random draws occurred before the neighboring initialization callback. The rejected implementation is absent from production. Diagnostic logs are saved under `build/audio-owner-lifetime-c-tests`, including `native-diagnostic.log`, `comdat-diagnostic.log`, and `const-diagnostic.log`.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_owner_lifetime.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_owner_lifetime_sanitized.ps1
```

The source audit checks the complete original destructor body against the C implementation, all stored fields, the unchanged constructor, the thin C++ destructor, and full project registrations. Existing migration audits explicitly follow the new C cleanup endpoint.

Runtime comparisons use the complete preceding constructor/destructor and the current constructor/C cleanup. Separate neighboring translation units observe startup and exit ordering. Each run exercises 256 seeded explicit owner lifetimes, both zero and nonzero initialization gates, provider mutation during termination, retained state, repeated initialization and all random draws. Storage is seeded after construction; cleanup is observed before the reference object's lifetime ends. A callback registered before the owner reports the trace after the neighboring exit callbacks and owner cleanup complete. The test copy of the actual C source adds only the same final observation as the reference destructor.

Verified checks:

- 20 Clang/GCC comparisons across five feature layouts and two optimization levels
- Six native MSVC comparisons across x86/x64 and three layouts, with `/GL`, `/LTCG` and `/OPT:REF`
- Two undefined-behavior and float-cast-overflow sanitizer comparisons, using trap mode
- 20 preceding owner-initialization runtime regressions
- Matching C/original C++/current C++ native ABI arrays: 164 entries each on x86 and x64
- 1,070 production-consumer compile checks and 10 configuration guards
- All 68 preceding migration source audits

The native ABI arrays cover overall size/alignment and every enabled field's offset, size and alignment. AddressSanitizer could not link because this local Clang installation lacks its runtime libraries; the available trap-based sanitizer checks passed.

No full game link or gameplay test was performed. The next lifetime step must replace compiler registration without changing observable startup/exit order, including link-time optimization. The audio owner still has its two thin C++ lifetime methods.
