$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$reflectionsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$reflectionsBuild=Join-Path $reflectionsRoot 'build/audio-reflections-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_reflections_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Reflection source audit failed'}
$reflectionsRuns=@()
foreach($reflectionsCompiler in @('clang','gcc')) {
 foreach($reflectionsOptimization in @('O0','O2')) {
  $reflectionsCpp=if($reflectionsCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($reflectionsShape in @('current','early','no-external','ps2','no-reflections')) {
   $reflectionsArgs=@("-I$reflectionsRoot/src/audio","-I$reflectionsRoot/src/core",'-include',"$reflectionsBuild/$reflectionsShape.h")
   & $reflectionsCompiler -std=c11 -pedantic-errors "-$reflectionsOptimization" @reflectionsArgs -c "$reflectionsRoot/src/audio/AudioReflections.c" -o "$reflectionsBuild/AudioReflections.o"
   if($LASTEXITCODE -ne 0) {throw 'Production C compile failed'}
   & $reflectionsCompiler -std=c11 -pedantic-errors "-$reflectionsOptimization" @reflectionsArgs -c "$reflectionsBuild/caller.c" -o "$reflectionsBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $reflectionsCpp -std=c++17 "-$reflectionsOptimization" @reflectionsArgs "$reflectionsBuild/before.cpp" -o "$reflectionsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $reflectionsCpp -std=c++17 "-$reflectionsOptimization" @reflectionsArgs "$reflectionsBuild/after.cpp" "$reflectionsBuild/AudioReflections.o" "$reflectionsBuild/caller.o" -o "$reflectionsBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $reflectionsBefore=& "$reflectionsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $reflectionsAfter=& "$reflectionsBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $reflectionsBefore -ne $reflectionsAfter) {throw "Reflection comparison failed: $reflectionsBefore / $reflectionsAfter"}
   $reflectionsRuns += "$reflectionsCompiler-$reflectionsOptimization-$reflectionsShape : $reflectionsAfter"
   Write-Output $reflectionsRuns[-1]
  }
 }
}
$reflectionsRuns | ConvertTo-Json | Set-Content "$reflectionsBuild/runtime-runs.json"
