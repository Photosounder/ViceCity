$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$collisionRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$collisionBuild=Join-Path $collisionRoot 'build/audio-collision-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_collision_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Collision fixture generation failed'}
$collisionArgs=@("-I$collisionRoot/src/audio", "-I$collisionRoot/src/core")
$collisionRuns=@()
foreach($collisionCompiler in @('clang','gcc')) {
 foreach($collisionOptimization in @('O0','O2')) {
  $collisionCpp=if($collisionCompiler -eq 'clang') {'clang++'} else {'g++'}
  & $collisionCpp -std=c++17 "-$collisionOptimization" @collisionArgs "$collisionBuild/before.cpp" -o "$collisionBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline collision compilation failed'}
  & $collisionCpp -std=c++17 "-$collisionOptimization" @collisionArgs -c "$collisionBuild/cpp-caller.cpp" -o "$collisionBuild/cpp-caller.o"
  if($LASTEXITCODE -ne 0) {throw 'Collision C++ caller compilation failed'}
  & $collisionCompiler -std=c11 -pedantic-errors "-$collisionOptimization" @collisionArgs "$collisionRoot/src/audio/AudioCollisionQueue.c" "$collisionRoot/src/audio/AudioCollisionMath.c" "$collisionBuild/after.c" "$collisionBuild/cpp-caller.o" -o "$collisionBuild/after.exe"
  if($LASTEXITCODE -ne 0) {throw 'Production collision C compilation/link failed'}
  $collisionExpected=& "$collisionBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline collision runtime failed'}
  $collisionActual=& "$collisionBuild/after.exe"
  if($LASTEXITCODE -ne 0 -or $collisionExpected -ne $collisionActual) {throw "Collision trace mismatch: $collisionExpected / $collisionActual"}
  $collisionRuns += "$collisionCompiler-$collisionOptimization : $collisionActual"
  Write-Output "Collision layouts, resets, and sorting traces matched: $($collisionRuns[-1])"
 }
}
$collisionRuns | ConvertTo-Json | Set-Content "$collisionBuild/runtime-runs.json"
