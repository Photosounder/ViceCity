# Time-step storage in C

`CTimeStep` and `TimeStep.cpp` are removed. `TimeStep.c` defines the same three float values, in the original definition order, initialized to `1.0f`. `TimeStep.h` declares them with C linkage for remaining C++ consumers. Save/load and rendering uses now refer directly to the C globals.

The save fragment remains three floats in this order: time step, frames per update, time scale. Both `GenericGameStorage.cpp` and the legacy `MemoryCard.cpp` retain their original surrounding statements, field order and four-byte cursor alignment. No bytes are added to the save file.

Run the quiet integration comparison with:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_timestep.ps1
```

Four Clang/GCC runs cover O0/O2. Two native MSVC runs cover Win32/Win64 with `/O2 /GL /LTCG /OPT:REF`. The comparison uses the original complete class and definitions as its baseline and the actual C storage with a separately compiled C11 caller. It checks initial values, distinct stable addresses, writes through retained C++ pointers, and 1,024 save/load fragments containing arbitrary float bits, signed zero, NaN payloads and infinity. The actual original save macros and field statements run against an unaligned guarded buffer. Expected bytes, 12-byte cursor advancement and loaded live storage are checked independently. Every run yields `354b223bb66ca815 fragments=1024 bytes=12`.

Native source checks compile all four existing consumers plus the C unit on both Windows architectures. Additional software reflection renderer compilations exercise the otherwise conditional movement use: 12 production source checks total. A complete source audit checks all eight original files, the removed C++ file, full caller transformations, initializers and Visual Studio project/filter rename. All 86 preceding migration source audits also pass.

CMake and Premake already collect C units. The Visual Studio project explicitly compiles the renamed unit as C11. No tests open windows, and these checks do not establish a full game link or gameplay result.

Forcing `PS2_MENU` in this Vice City tree causes eight unrelated `MemoryCard.cpp` errors involving GTA III level names and `CStreaming::HaveAllBigBuildingsLoaded`. Compiling the immutable original with its original `CTimeStep` header produces exactly the same eight errors. The legacy save fragment itself is covered by its unchanged macro definitions and complete caller audit; the unsupported menu configuration is not reported as a passing build.
