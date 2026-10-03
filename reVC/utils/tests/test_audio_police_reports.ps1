$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$controlRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$controlBuild=Join-Path $controlRoot 'build/audio-police-reports-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_police_reports_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Control fixture generation failed'}
$controlArgs=@("-I$controlBuild", "-I$controlRoot/src/audio", "-I$controlRoot/src/core")
$controlRuns=@()
foreach($controlCompiler in @('clang','gcc')) {
 foreach($controlOptimization in @('O0','O2')) {
  foreach($controlShape in @('default','vanilla','no-external','ps2')) {
  $controlShapeArgs=@('-include', "$controlBuild/$controlShape.h")
  $controlCpp=if($controlCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($providerModule in @('AudioPoliceState','AudioPoliceReports','AudioCrimes')) {
    & $controlCompiler -std=c11 -pedantic-errors "-$controlOptimization" @controlArgs @controlShapeArgs -c "$controlRoot/src/audio/$providerModule.c" -o "$controlBuild/$providerModule.o"
    if($LASTEXITCODE -ne 0) {throw "C compilation failed: $providerModule"}
  }
  & $controlCompiler -std=c11 -pedantic-errors "-$controlOptimization" @controlArgs @controlShapeArgs -c "$controlBuild/caller.c" -o "$controlBuild/caller.o"
  if($LASTEXITCODE -ne 0) {throw 'C caller compilation failed'}
  & $controlCpp -std=c++17 "-$controlOptimization" @controlArgs @controlShapeArgs "$controlBuild/before.cpp" "$controlBuild/AudioPoliceState.o" "$controlBuild/AudioCrimes.o" "$controlBuild/caller.o" -o "$controlBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline control compilation failed'}
  & $controlCpp -std=c++17 "-$controlOptimization" @controlArgs @controlShapeArgs "$controlBuild/after.cpp" "$controlBuild/AudioPoliceState.o" "$controlBuild/AudioPoliceReports.o" "$controlBuild/AudioCrimes.o" "$controlBuild/caller.o" -o "$controlBuild/after.exe"
  if($LASTEXITCODE -ne 0) {throw 'Mixed control C/C++ link failed'}
  $controlExpected=& "$controlBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline control runtime failed'}
  $controlActual=& "$controlBuild/after.exe"
  if($LASTEXITCODE -ne 0 -or $controlExpected -ne $controlActual) {throw "Control trace mismatch: $controlExpected / $controlActual"}
  $controlRuns += "$controlCompiler-$controlOptimization-$controlShape : $controlActual"
  Write-Output "Police report trace matched: $($controlRuns[-1])"
  }
 }
}
$controlRuns | ConvertTo-Json | Set-Content "$controlBuild/runtime-runs.json"
