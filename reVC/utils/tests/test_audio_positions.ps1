$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$positionsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$positionsBuild=Join-Path $positionsRoot 'build/audio-positions-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_positions_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Positions fixture generation failed'}
$positionsArgs=@("-I$positionsBuild", "-I$positionsRoot/src/audio", "-I$positionsRoot/src/core")
$positionsRuns=@()
foreach($positionsCompiler in @('clang','gcc')) {
 foreach($positionsOptimization in @('O0','O2')) {
  foreach($positionsShape in @('current','early')) {
  $positionsShapeArgs=@('-include', "$positionsBuild/$positionsShape.h")
  $positionsCpp=if($positionsCompiler -eq 'clang') {'clang++'} else {'g++'}
  & $positionsCpp -std=c++17 "-$positionsOptimization" @positionsArgs @positionsShapeArgs "$positionsBuild/before.cpp" -o "$positionsBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline positions compilation failed'}
  & $positionsCompiler -std=c11 -pedantic-errors "-$positionsOptimization" @positionsArgs @positionsShapeArgs -c "$positionsRoot/src/audio/AudioMissionPosition.c" -o "$positionsBuild/positionss.o"
  if($LASTEXITCODE -ne 0) {throw 'Production positions C compilation failed'}
  & $positionsCompiler -std=c11 -pedantic-errors "-$positionsOptimization" @positionsArgs @positionsShapeArgs -c "$positionsBuild/caller.c" -o "$positionsBuild/caller.o"
  if($LASTEXITCODE -ne 0) {throw 'Positions C caller compilation failed'}
  & $positionsCpp -std=c++17 "-$positionsOptimization" @positionsArgs @positionsShapeArgs "$positionsBuild/after.cpp" "$positionsBuild/positionss.o" "$positionsBuild/caller.o" -o "$positionsBuild/after.exe"
  if($LASTEXITCODE -ne 0) {throw 'Mixed positions C/C++ link failed'}
  $positionsExpected=& "$positionsBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline positions runtime failed'}
  $positionsActual=& "$positionsBuild/after.exe"
  if($LASTEXITCODE -ne 0 -or $positionsExpected -ne $positionsActual) {throw "Positions trace mismatch: $positionsExpected / $positionsActual"}
  $positionsRuns += "$positionsCompiler-$positionsOptimization-$positionsShape : $positionsActual"
  Write-Output "Mission positions and reflection traces matched: $($positionsRuns[-1])"
  }
 }
}
$positionsRuns | ConvertTo-Json | Set-Content "$positionsBuild/runtime-runs.json"
