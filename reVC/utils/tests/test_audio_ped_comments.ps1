$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$pedRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$pedBuild=Join-Path $pedRoot 'build/audio-ped-comments-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_ped_comments_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Ped fixture generation failed'}
$pedArgs=@("-I$pedRoot/src/audio", "-I$pedRoot/src/core")
$pedRuns=@()
foreach($pedCompiler in @('clang','gcc')) {
 foreach($pedOptimization in @('O0','O2')) {
  foreach($pedShape in @('default','vanilla','no-external','ps2')) {
  $pedShapeArgs=@('-include', "$pedBuild/$pedShape.h")
  $pedCpp=if($pedCompiler -eq 'clang') {'clang++'} else {'g++'}
  & $pedCpp -std=c++17 "-$pedOptimization" @pedArgs @pedShapeArgs "$pedBuild/before.cpp" -o "$pedBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline ped compilation failed'}
  & $pedCompiler -std=c11 -pedantic-errors "-$pedOptimization" @pedArgs @pedShapeArgs -c "$pedRoot/src/audio/AudioPedComments.c" -o "$pedBuild/peds.o"
  if($LASTEXITCODE -ne 0) {throw 'Production ped C compilation failed'}
  & $pedCompiler -std=c11 -pedantic-errors "-$pedOptimization" @pedArgs @pedShapeArgs -c "$pedBuild/caller.c" -o "$pedBuild/caller.o"
  if($LASTEXITCODE -ne 0) {throw 'Ped C caller compilation failed'}
  & $pedCpp -std=c++17 "-$pedOptimization" @pedArgs @pedShapeArgs "$pedBuild/after.cpp" "$pedBuild/peds.o" "$pedBuild/caller.o" -o "$pedBuild/after.exe"
  if($LASTEXITCODE -ne 0) {throw 'Mixed ped C/C++ link failed'}
  $pedExpected=& "$pedBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline ped runtime failed'}
  $pedActual=& "$pedBuild/after.exe"
  if($LASTEXITCODE -ne 0 -or $pedExpected -ne $pedActual) {throw "Ped trace mismatch: $pedExpected / $pedActual"}
  $pedRuns += "$pedCompiler-$pedOptimization-$pedShape : $pedActual"
  Write-Output "Pedestrian queue layout, ordering, and carryover matched: $($pedRuns[-1])"
  }
 }
}
$pedRuns | ConvertTo-Json | Set-Content "$pedBuild/runtime-runs.json"
