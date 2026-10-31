$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$stepRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$stepBuild=Join-Path $stepRoot 'build/timestep-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/timestep_fixture.py"
if($LASTEXITCODE -ne 0){throw 'Time-step source audit failed'}
$stepRuns=@()
foreach($stepCompiler in @('clang','gcc')) {
 foreach($stepOptimization in @('O0','O2')) {
  $stepCpp=if($stepCompiler -eq 'clang') {'clang++'} else {'g++'}
  $stepArgs=@("-I$stepBuild","-I$stepRoot/src/core")
  & $stepCompiler -std=c11 -pedantic-errors "-$stepOptimization" @stepArgs -c "$stepRoot/src/core/TimeStep.c" -o "$stepBuild/TimeStep.o"
  if($LASTEXITCODE -ne 0){throw 'C time-step storage compile failed'}
  & $stepCompiler -std=c11 -pedantic-errors "-$stepOptimization" @stepArgs -c "$stepBuild/caller.c" -o "$stepBuild/caller.o"
  if($LASTEXITCODE -ne 0){throw 'C save caller compile failed'}
  & $stepCpp -std=c++17 "-$stepOptimization" @stepArgs "$stepBuild/before.cpp" -o "$stepBuild/before.exe"
  if($LASTEXITCODE -ne 0){throw 'Original storage compile failed'}
  & $stepCpp -std=c++17 "-$stepOptimization" @stepArgs "$stepBuild/after.cpp" "$stepBuild/TimeStep.o" "$stepBuild/caller.o" -o "$stepBuild/after.exe"
  if($LASTEXITCODE -ne 0){throw 'Mixed C/C++ link failed'}
  $stepBefore=& "$stepBuild/before.exe"
  if($LASTEXITCODE -ne 0){throw 'Original save fragment failed'}
  $stepAfter=& "$stepBuild/after.exe"
  if($LASTEXITCODE -ne 0 -or $stepBefore -ne $stepAfter){throw "Time-step comparison failed: $stepBefore / $stepAfter"}
  $stepRuns += "$stepCompiler-$stepOptimization : $stepAfter"
  Write-Output $stepRuns[-1]
 }
}
$stepRuns | ConvertTo-Json | Set-Content "$stepBuild/runtime-runs.json"
