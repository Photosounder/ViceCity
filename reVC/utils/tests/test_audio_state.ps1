$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$stateRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$stateBuild=Join-Path $stateRoot 'build/audio-state-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_state_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Audio state fixture generation failed'}
$stateArgs=@("-I$stateRoot/src/audio", "-I$stateRoot/src/core")
$stateRuns=@()
foreach($stateCompiler in @('clang','gcc')) {
 foreach($stateOptimization in @('O0','O2')) {
  $stateCpp=if($stateCompiler -eq 'clang') {'clang++'} else {'g++'}
  & $stateCpp -std=c++17 "-$stateOptimization" @stateArgs "$stateBuild/before.cpp" -o "$stateBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline audio state compilation failed'}
  $stateExpected=& "$stateBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline audio state runtime failed'}
  foreach($stateCaller in @('C','C++')) {
   if($stateCaller -eq 'C') {
    & $stateCompiler -std=c11 -pedantic-errors "-$stateOptimization" @stateArgs "$stateBuild/after.c" -o "$stateBuild/after.exe"
   } else {
    & $stateCpp -std=c++17 -x c++ "-$stateOptimization" @stateArgs "$stateBuild/after.c" -o "$stateBuild/after.exe"
   }
   if($LASTEXITCODE -ne 0) {throw 'C audio state compilation failed'}
   $stateActual=& "$stateBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $stateExpected -ne $stateActual) {throw "Audio state trace mismatch: $stateExpected / $stateActual"}
   $stateRuns += "$stateCompiler-$stateOptimization-$stateCaller : $stateActual"
   Write-Output "Initialization bytes, layouts, and queue traces matched: $($stateRuns[-1])"
  }
 }
}
$stateRuns | ConvertTo-Json | Set-Content "$stateBuild/runtime-runs.json"
