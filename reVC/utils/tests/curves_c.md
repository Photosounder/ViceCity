# Curve algorithms in C11

`CCurves` is removed and `Curves.cpp` is renamed to `Curves.c`. Both complete algorithms now use ordinary C functions, explicit float arithmetic and a three-float `CurvesVector` record. The original `FIX_BUGS` coefficient comparisons, approximate `3.1415f` constant, planar math, interpolation expressions, time conversion and position-before-direction output order are retained.

All six speed-scale calls and both curve-point calls are converted. Existing C++ callers copy named `CVector` fields through the header's small conversion helper, pass C values, and copy the two point results back in the original order. No casts assume the C++ vector layout. Both algorithms and their vector arithmetic execute in C; `CVector` itself remains a separate conversion task.

Run the quiet tests from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_curves.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_curves.ps1 -Sanitized
```

The baseline compiles the complete original owner, original `Vector.h` and `Vector2D.h` operations, float math wrappers and actual RenderWare vector declaration from immutable snapshots. The converted model calls the actual C kernel through a separately compiled C11 caller and exercises the same coordinate-copy pattern as production. Each run observes 65,800 groups and all six input/output vectors. Coverage includes all 36 C++ output alias combinations, reversed and coincident points, zero directions, interpolation outside the usual interval, zero/negative durations, signed zero, subnormal values, infinities and NaNs.

Independent checks cover the exact `0.9f` dot-product boundary, a known straight curve and both coefficient equalities that differ between `FIX_BUGS` and the original behavior. Default and vanilla profiles each run with Clang/GCC at O0/O2 and native Win32/Win64 MSVC at `/O2 /GL /LTCG /OPT:REF`: 12 comparisons. Two Clang O1 runs enable undefined-behavior and float-cast-overflow trapping. Native checks compile both actual consumers and the actual C unit under both profiles and architectures: 12 production source compilations.

Finite numbers, infinities and signed zeros compare by their exact float bits. NaNs compare by classification. Raw tracing of 1,250,200 words diagnosed the initially different hashes: GCC O0 differed in 24 words and native Win32 MSVC in 116, all consisting of NaN sign changes in extreme/overflow cases. Instrumenting MSVC also changed some NaN signs without changing its operations. Neither trace found any differing non-NaN value. The tests retain those cases rather than dropping them; matching NaN sign/payload bits is not a verified property of this conversion.

Clang, GCC and native Win64 comparison hashes are `f2b3f0095ecb6925` for default and `8e1890a6e97a6093` for vanilla. Native Win32 hashes are `7b6b27811e92e1e5` and `404504f9e6dcc74b`; each matches its original C++ model on that target. All runs observe `groups=65800`.

The source audit checks every complete algorithm statement, dependency snapshot, all eight calls, all six changed existing files, removal of the C++ unit and the Visual Studio project/filter rename. All 87 earlier source audits pass. Existing CMake/Premake globs include C; Visual Studio explicitly compiles the new unit as C11. No tests open windows. Full game linking and gameplay are not established by these checks.
