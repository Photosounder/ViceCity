$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$soundRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$soundBuild=Join-Path $soundRoot 'build/audio-sound-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_sound_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Sound fixture generation failed'}
$soundArgs=@("-I$soundBuild", "-I$soundRoot/src/audio", "-I$soundRoot/src/core")
$soundRuns=@()
foreach($soundCompiler in @('clang','gcc')) {
 foreach($soundOptimization in @('O0','O2')) {
  foreach($soundShape in @('default','vanilla','no-external','no-reflections','ps2','ps2-reverb')) {
  $soundShapeArgs=@('-include', "$soundBuild/$soundShape.h")
  $soundCpp=if($soundCompiler -eq 'clang') {'clang++'} else {'g++'}
  & $soundCpp -std=c++17 "-$soundOptimization" @soundArgs @soundShapeArgs "$soundBuild/before.cpp" -o "$soundBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline sound compilation failed'}
  & $soundCompiler -std=c11 -pedantic-errors "-$soundOptimization" @soundArgs @soundShapeArgs -c "$soundRoot/src/audio/AudioSoundQueue.c" -o "$soundBuild/sounds.o"
  if($LASTEXITCODE -ne 0) {throw 'Production sound C compilation failed'}
  & $soundCompiler -std=c11 -pedantic-errors "-$soundOptimization" @soundArgs @soundShapeArgs -c "$soundBuild/caller.c" -o "$soundBuild/caller.o"
  if($LASTEXITCODE -ne 0) {throw 'Sound C caller compilation failed'}
  & $soundCpp -std=c++17 "-$soundOptimization" @soundArgs @soundShapeArgs "$soundBuild/after.cpp" "$soundBuild/sounds.o" "$soundBuild/caller.o" -o "$soundBuild/after.exe"
  if($LASTEXITCODE -ne 0) {throw 'Mixed sound C/C++ link failed'}
  $soundExpected=& "$soundBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline sound runtime failed'}
  $soundActual=& "$soundBuild/after.exe"
  if($LASTEXITCODE -ne 0 -or $soundExpected -ne $soundActual) {throw "Sound trace mismatch: $soundExpected / $soundActual"}
  $soundRuns += "$soundCompiler-$soundOptimization-$soundShape : $soundActual"
  Write-Output "Sound record layout, vector boundaries, and queue ordering matched: $($soundRuns[-1])"
  }
 }
}
$soundRuns | ConvertTo-Json | Set-Content "$soundBuild/runtime-runs.json"
