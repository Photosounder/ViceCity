$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$geometryRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$geometryBuild=Join-Path $geometryRoot 'build/audio-geometry-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_geometry_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Geometry source audit failed'}
$geometryRuns=@()
foreach($geometryCompiler in @('clang','gcc')) {
 foreach($geometryOptimization in @('O0','O2')) {
  $geometryCpp=if($geometryCompiler -eq 'clang') {'clang++'} else {'g++'}
  & $geometryCompiler -std=c11 -pedantic-errors "-$geometryOptimization" "-I$geometryRoot/src/audio" -c "$geometryRoot/src/audio/AudioGeometry.c" -o "$geometryBuild/math.o"
  if($LASTEXITCODE -ne 0) {throw 'Production C compile failed'}
  & $geometryCompiler -std=c11 -pedantic-errors "-$geometryOptimization" "-I$geometryRoot/src/audio" -c "$geometryBuild/caller.c" -o "$geometryBuild/caller.o"
  if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
  & $geometryCpp -std=c++17 "-$geometryOptimization" "-I$geometryRoot/src/audio" "$geometryBuild/compare.cpp" "$geometryBuild/math.o" "$geometryBuild/caller.o" -o "$geometryBuild/compare.exe"
  if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
  $geometryTrace=& "$geometryBuild/compare.exe"
  if($LASTEXITCODE -ne 0) {throw 'Geometry comparison failed'}
  $geometryRuns += "$geometryCompiler-$geometryOptimization : $geometryTrace"
  Write-Output $geometryRuns[-1]
 }
}
$geometryRuns | ConvertTo-Json | Set-Content "$geometryBuild/runtime-runs.json"
