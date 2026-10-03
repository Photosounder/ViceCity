# Cached audio distance conversion

`AudioMath_CalculateDistance` is compiled as C11 in the existing AudioMath.c translation unit. The CalculateDistance owner member and declaration were removed, and all 34 callers in AudioLogic.cpp and AudioCollision.cpp now pass the flag and cached float explicitly. Owner fields and camera transforms are unchanged.

The original member calls cAudioManager::Sqrt, which clamps inputs <= 0 to positive zero and otherwise calls sqrtf. The C implementation retains that clamp, the false-flag gate, distance-before-flag write order, and the value passed by the collision caller when its input and output use the same field.

Verification:

- Independent statement audit against saved original source, including the original square root wrapper and every caller edit
- 1,062,912 comparisons per configuration, with all 256 flag bytes, signed zero, negative values, infinities, quiet NaNs, finite exponent ranges, cached sentinel bits, and repeated calls
- Clang and GCC at O0/O2, and native MSVC x86/x64: identical digest 1835530974922868202
- Original six AudioMath kernels retain digest 10103914456804643557 in Clang/GCC O0/O2 regressions
- All 19 earlier audio source audits pass
- Native MSVC x86/x64 compilation of AudioManager.cpp, AudioLogic.cpp, AudioCollision.cpp and AudioMath.c
- All 400 consumer compilation checks pass across x86/x64, D3D9/software, threaded/single and decoder configurations; results are recorded in build/audio-distance-c-tests/consumer-runs.json

Full game linking and playback were not tested. The next candidate is GetDistanceSquared and TranslateEntity, with a narrow camera adapter and checks for matrix convention and aliasing before conversion.
