$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$lifecycleRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$lifecycleBuild=Join-Path $lifecycleRoot 'build/audio-lifecycle-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_lifecycle_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Lifecycle source audit failed'}
$lifecycleRuns=@()
foreach($lifecycleCompiler in @('clang','gcc')) {
 foreach($lifecycleOptimization in @('O0','O2')) {
  $lifecycleCpp=if($lifecycleCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($lifecycleShape in @('default','vanilla','no-external','ps2')) {
   $lifecycleArgs=@("-I$lifecycleRoot/src/audio","-I$lifecycleRoot/src/core",'-include',"$lifecycleBuild/$lifecycleShape.h")
   foreach($lifecycleUnit in @('AudioLifecycle','AudioPoliceState')) {
    & $lifecycleCompiler -std=c11 -pedantic-errors "-$lifecycleOptimization" @lifecycleArgs -c "$lifecycleRoot/src/audio/$lifecycleUnit.c" -o "$lifecycleBuild/$lifecycleUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $lifecycleUnit"}
   }
   & $lifecycleCompiler -std=c11 -pedantic-errors "-$lifecycleOptimization" @lifecycleArgs -c "$lifecycleBuild/caller.c" -o "$lifecycleBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $lifecycleCpp -std=c++17 "-$lifecycleOptimization" @lifecycleArgs "$lifecycleBuild/before.cpp" "$lifecycleBuild/AudioPoliceState.o" -o "$lifecycleBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $lifecycleCpp -std=c++17 "-$lifecycleOptimization" @lifecycleArgs "$lifecycleBuild/after.cpp" "$lifecycleBuild/AudioPoliceState.o" "$lifecycleBuild/AudioLifecycle.o" "$lifecycleBuild/caller.o" -o "$lifecycleBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $lifecycleBefore=& "$lifecycleBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $lifecycleAfter=& "$lifecycleBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $lifecycleBefore -ne $lifecycleAfter) {throw "Lifecycle comparison failed: $lifecycleBefore / $lifecycleAfter"}
   $lifecycleRuns += "$lifecycleCompiler-$lifecycleOptimization-$lifecycleShape : $lifecycleAfter"
   Write-Output $lifecycleRuns[-1]
  }
 }
}
$lifecycleRuns | ConvertTo-Json | Set-Content "$lifecycleBuild/runtime-runs.json"
