# Local Visual Studio build

1. Run `premake-vs2022.cmd` from this checkout. It requires Python 3 and generates
   the solution with the Visual Studio 2022 v143 toolset.
2. Open `build/reVC.sln` in Visual Studio 2022.
3. Select `Debug` or `Release` and platform `win-amd64-librw_d3d9-oal`.
4. Build the solution. The executable goes under
   `bin/win-amd64-librw_d3d9-oal/Debug` or `Release`.

The `build` directory contains generated files. If it is removed, rerun
`premake-vs2022.cmd`. Regenerate after source files are added or renamed.
The generator sets C11 compilation for C files and distinct object filenames
where C and C++ files share a name. It uses local rouziclib and CITAlloc installations under `C:/msys/home` and
the 64-bit SDL2 SDK in `D:/VC_libs` (`include` and `lib`). A 32-bit build
requires a separate matching 32-bit SDL2 SDK. Regenerating the solution
overwrites manual changes to generated project files, including output paths.

From a shell with MSBuild available, the equivalent Debug build is:

```bat
msbuild build\reVC.sln /m /p:Configuration=Debug /p:Platform=win-amd64-librw_d3d9-oal
```

The bundled Premake supports the `vs2019` generator format; the new command
explicitly selects v143, so the generated solution builds with Visual Studio
2022 without installing the older v142 compiler. Git hash generation is disabled.
