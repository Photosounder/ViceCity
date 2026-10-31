$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$generalRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$generalBuild=Join-Path $generalRoot 'build/general-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/general_fixture.py"
if($LASTEXITCODE -ne 0){throw 'General source audit failed'}
$generalRuns=@()
foreach($generalCompiler in @('clang')) {
 foreach($generalOptimization in @('O1')) {
  $generalCpp=if($generalCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($generalShape in @('default','regular')) {
   $generalFolder=Join-Path $generalBuild $generalShape
   $generalArgs=@('-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$generalFolder","-I$generalBuild","-I$generalRoot/src/core",'-include',"$generalFolder/shape.h")
   & $generalCompiler -std=c11 -pedantic-errors "-$generalOptimization" @generalArgs -c "$generalRoot/src/core/General.c" -o "$generalFolder/General.o"
   if($LASTEXITCODE -ne 0){throw 'General C compile failed'}
   & $generalCompiler -std=c11 -pedantic-errors "-$generalOptimization" @generalArgs -c "$generalBuild/caller.c" -o "$generalFolder/caller.o"
   if($LASTEXITCODE -ne 0){throw 'General C caller failed'}
   & $generalCpp -std=c++17 "-$generalOptimization" @generalArgs "$generalBuild/before.cpp" -o "$generalFolder/before.exe"
   if($LASTEXITCODE -ne 0){throw 'Original General compile failed'}
   & $generalCpp -std=c++17 "-$generalOptimization" @generalArgs "$generalBuild/after.cpp" "$generalFolder/General.o" "$generalFolder/caller.o" -o "$generalFolder/after.exe"
   if($LASTEXITCODE -ne 0){throw 'General mixed link failed'}
   $generalBefore=& "$generalFolder/before.exe"
   if($LASTEXITCODE -ne 0){throw 'Original General runtime failed'}
   $generalAfter=& "$generalFolder/after.exe"
   if($LASTEXITCODE -ne 0 -or $generalBefore -ne $generalAfter){throw "General comparison failed: $generalBefore / $generalAfter"}
   $generalRuns += "$generalCompiler-$generalOptimization-$generalShape : $generalAfter"
   Write-Output $generalRuns[-1]
  }
 }
}
$generalRuns | ConvertTo-Json | Set-Content "$generalBuild/sanitized-runs.json"
