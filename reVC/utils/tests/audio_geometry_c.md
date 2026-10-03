# Audio camera geometry conversion

AudioGeometry.c owns the original weighted distance calculation and inverse matrix multiply as C11. The two cAudioManager methods and two mission camera wrappers were removed. Mission playback calls C directly; game vector consumers use coordinate-copy adapters from AudioSoundGame.h. Camera field access remains in two narrow C++ callbacks in AudioLogic.cpp.

The original distance formula squares x/y differences and a z difference multiplied by 0.2f. The original inverse multiply subtracts camera translation before taking dot products with right, forward and up. The new C module retains these formulas even for nonorthogonal matrices; it does not apply a general inverse or normalize vectors. Translation computes a temporary result before writing the output, preserving in-place use.

The new .c and .h are registered in the existing Visual Studio project and filters. CMake and premake already discover .c files and use C11.

Verification:

- Independent statement audits against the saved owner distance body and actual Matrix.h MultiplyInverse definition
- Complete caller, host adapter and header edit audits; all 20 earlier source audits pass
- 50,060 runtime cases in each of six configurations: Clang/GCC O0/O2 and native MSVC x86/x64
- Identical digest 1741264880467042197, including identity, translation, rotation, reflected axes, nonorthogonal and zero matrices, finite randomized matrices, signed zeros, subnormals, extreme floats, infinities and quiet NaNs
- Separate/in-place C position and game-vector outputs, exact float bits, and exactly one corresponding camera lookup per operation
- 410 unique consumer compilation checks covering x86/x64, D3D9/software, threading and decoder configurations
- Native x86/x64 compilation of actual AudioManager.cpp, AudioLogic.cpp, AudioCollision.cpp, MusicManager.cpp and AudioGeometry.c
- All 16 mission playback before/after regressions retain digest 17102462808055070296; their geometry callbacks stay controlled while this fixture tests actual production C geometry independently

Full game linking and playback remain untested. Renderer matrices and camera construction were not changed.
