$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$entityRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$entityBuild=Join-Path $entityRoot 'build/audio-entity-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_entity_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Entity fixture generation failed'}
$entityArgs=@("-I$entityRoot/src/audio", "-I$entityRoot/src/core")
$entityRuns=@()
foreach($entityCompiler in @('clang','gcc')) {
 foreach($entityOptimization in @('O0','O2')) {
  foreach($entityShape in @('default','vanilla','no-external','ps2')) {
  $entityShapeArgs=@('-include', "$entityBuild/$entityShape.h")
  $entityCpp=if($entityCompiler -eq 'clang') {'clang++'} else {'g++'}
  & $entityCpp -std=c++17 "-$entityOptimization" @entityArgs @entityShapeArgs "$entityBuild/before.cpp" -o "$entityBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline entity compilation failed'}
  & $entityCompiler -std=c11 -pedantic-errors "-$entityOptimization" @entityArgs @entityShapeArgs -c "$entityRoot/src/audio/AudioEntities.c" -o "$entityBuild/entitys.o"
  if($LASTEXITCODE -ne 0) {throw 'Production entity C compilation failed'}
  & $entityCompiler -std=c11 -pedantic-errors "-$entityOptimization" @entityArgs @entityShapeArgs -c "$entityBuild/caller.c" -o "$entityBuild/caller.o"
  if($LASTEXITCODE -ne 0) {throw 'Entity C caller compilation failed'}
  & $entityCpp -std=c++17 "-$entityOptimization" @entityArgs @entityShapeArgs "$entityBuild/after.cpp" "$entityBuild/entitys.o" "$entityBuild/caller.o" -o "$entityBuild/after.exe"
  if($LASTEXITCODE -ne 0) {throw 'Mixed entity C/C++ link failed'}
  $entityExpected=& "$entityBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline entity runtime failed'}
  $entityActual=& "$entityBuild/after.exe"
  if($LASTEXITCODE -ne 0 -or $entityExpected -ne $entityActual) {throw "Entity trace mismatch: $entityExpected / $entityActual"}
  $entityRuns += "$entityCompiler-$entityOptimization-$entityShape : $entityActual"
  Write-Output "Entity lifecycle, status, and event ordering matched: $($entityRuns[-1])"
  }
 }
}
$entityRuns | ConvertTo-Json | Set-Content "$entityBuild/runtime-runs.json"
