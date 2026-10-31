# General helpers in C11

`src/core/General.c` implements all 15 former inline `CGeneral` functions. `General.h` is a C header with C linkage for remaining C++ consumers. The class and overloaded public API are removed.

The two range functions are `General_GetRandomNumberInRangeFloat` and `General_GetRandomNumberInRangeInt`. Both preserve the original floating-point formula and one random draw, including equal or reversed bounds. The integer result truncates toward zero. Compiler diagnostics on immutable copies selected the original overload at all 627 active call sites, including `FIX_BUGS` alternatives, screen droplets, PC water and software rendering. Calls on the same line were separated in diagnostic copies because MSVC suppresses repeated deprecation warnings on one line. No diagnostic declarations enter production.

`General_SolveQuadratic` takes two float pointers. A negative discriminant leaves both outputs untouched; the second output is written before the first, preserving aliases. String helpers retain their Boolean difference convention and the asymmetric `faststrncmp` prefix behavior. Angle formulas, float math functions, loop boundaries, constants, clamping and the original zero-coordinate adjustment are preserved.

Run the quiet tests from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_general.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_general_sanitized.ps1
```

The fixtures compare the complete original class with a separately compiled C11 kernel and C11 caller. Each run covers 131,362 groups, 786,435 random draws and 256 reseeds. Comparisons record exact float bits, integer results, both quadratic outputs, aliases, RNG seed words and callback counts. Inputs include all quadrants, signed zero, subnormal and extreme floats, NaNs, infinities where the original operation terminates, ASCII strings, counts, equal/reversed bounds and negative seeds. Independent checks cover masks, negative-range truncation, draw counts, degree/radian boundaries and the quadratic/string quirks.

Eight Clang/GCC comparisons cover O0/O2 and both RNG seed widths. Eight native MSVC comparisons cover Win32/Win64, `/O2 /GL /LTCG /OPT:REF`, both RNG widths and ASCII/ctype comparisons. Two Clang runs enable undefined-behavior and float-cast-overflow trapping. NaN payload/sign hashes can differ between compilers and optimization levels; each C result must equal the original C++ result built with the same compiler/options.

`general_fixture.py` audits every complete transformation against `build/general-c-tests/before`, all 90 changed existing files, the full C kernel and all compiler-selected overload arguments. Native source checks compile 87 actual C++ consumers and the C kernel for each Windows architecture: 176 production source compilations. All 85 preceding migration source audits also pass.

The new unit is registered as C11 in the Visual Studio project and its filter. Existing CMake and Premake source globs already include C files. Tests do not launch the game or validate a full game link or gameplay.

Additional native Win32/Win64 checks compile nine real consumers with optional features enabled and eight with `FIX_BUGS` disabled. These add 34 source compilations, for 210 production source checks in total. Both final C objects were also refreshed after formatting the kernel.

One additional diagnostic profile, PC water disabled together with `FIX_BUGS`, exposed an existing `WaterLevel.cpp` call to the `FIX_BUGS`-only `Timer_GetTimeStepFix`. The original timer header also guarded that method with `FIX_BUGS`; this is independent of the general-helper conversion. Overloads in PC water were checked under its enabled profile, which compiled successfully.
