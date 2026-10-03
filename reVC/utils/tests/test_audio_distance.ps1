$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$distanceRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$distanceBuild=Join-Path $distanceRoot 'build/audio-distance-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_distance_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Distance source audit failed'}
$distanceRuns=@()
foreach($distanceCompiler in @('clang','gcc')) {
 foreach($distanceOptimization in @('O0','O2')) {
  $distanceCpp=if($distanceCompiler -eq 'clang') {'clang++'} else {'g++'}
  & $distanceCompiler -std=c11 -pedantic-errors "-$distanceOptimization" "-I$distanceRoot/src/audio" -c "$distanceRoot/src/audio/AudioMath.c" -o "$distanceBuild/math.o"
  if($LASTEXITCODE -ne 0) {throw 'Production C compile failed'}
  & $distanceCompiler -std=c11 -pedantic-errors "-$distanceOptimization" "-I$distanceRoot/src/audio" -c "$distanceBuild/caller.c" -o "$distanceBuild/caller.o"
  if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
  & $distanceCpp -std=c++17 "-$distanceOptimization" "$distanceBuild/compare.cpp" "$distanceBuild/math.o" "$distanceBuild/caller.o" -o "$distanceBuild/compare.exe"
  if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
  $distanceTrace=& "$distanceBuild/compare.exe"
  if($LASTEXITCODE -ne 0) {throw 'Distance comparison failed'}
  $distanceRuns += "$distanceCompiler-$distanceOptimization : $distanceTrace"
  Write-Output $distanceRuns[-1]
 }
}
$distanceRuns | ConvertTo-Json | Set-Content "$distanceBuild/runtime-runs.json"
