$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$serviceRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$serviceBuild=Join-Path $serviceRoot 'build/audio-service-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_service_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Service source audit failed'}
$serviceRuns=@()
foreach($serviceCompiler in @('clang','gcc')) {
 foreach($serviceOptimization in @('O0','O2')) {
  $serviceCpp=if($serviceCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($serviceShape in @('default','vanilla','no-reflections','no-external','ps2')) {
   $serviceArgs=@("-I$serviceRoot/src/audio","-I$serviceRoot/src/core",'-include',"$serviceBuild/$serviceShape.h")
   foreach($serviceUnit in @('AudioService','AudioMissionState','AudioMissionPosition')) {
    & $serviceCompiler -std=c11 -pedantic-errors "-$serviceOptimization" @serviceArgs -c "$serviceRoot/src/audio/$serviceUnit.c" -o "$serviceBuild/$serviceUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $serviceUnit"}
   }
   & $serviceCompiler -std=c11 -pedantic-errors "-$serviceOptimization" @serviceArgs -c "$serviceBuild/random.c" -o "$serviceBuild/random.o"
   if($LASTEXITCODE -ne 0) {throw 'C RNG unit compile failed'}
   & $serviceCompiler -std=c11 -pedantic-errors "-$serviceOptimization" @serviceArgs -c "$serviceBuild/caller.c" -o "$serviceBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $serviceCommon=@("$serviceBuild/random.o","$serviceBuild/AudioMissionState.o","$serviceBuild/AudioMissionPosition.o")
   & $serviceCpp -std=c++17 "-$serviceOptimization" @serviceArgs "$serviceBuild/before.cpp" @serviceCommon -o "$serviceBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $serviceCpp -std=c++17 "-$serviceOptimization" @serviceArgs "$serviceBuild/after.cpp" @serviceCommon "$serviceBuild/AudioService.o" "$serviceBuild/caller.o" -o "$serviceBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $serviceBefore=& "$serviceBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $serviceAfter=& "$serviceBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $serviceBefore -ne $serviceAfter) {throw "Service comparison failed: $serviceBefore / $serviceAfter"}
   $serviceRuns += "$serviceCompiler-$serviceOptimization-$serviceShape : $serviceAfter"
   Write-Output $serviceRuns[-1]
  }
 }
}
$serviceRuns | ConvertTo-Json | Set-Content "$serviceBuild/runtime-runs.json"
