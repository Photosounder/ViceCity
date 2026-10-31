$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$vehicleEngineRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$vehicleEngineBuild=Join-Path $vehicleEngineRoot 'build/audio-vehicle-engine-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_vehicle_engine_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'VehicleEngine source audit failed'}
$vehicleEngineRuns=@()
foreach($vehicleEngineCompiler in @('clang','gcc')) {
 foreach($vehicleEngineOptimization in @('O0','O2')) {
  $vehicleEngineCpp=if($vehicleEngineCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($vehicleEngineShape in @('default','vanilla','no-reverb','no-external','ps2')) {
   $vehicleEngineArgs=@("-I$vehicleEngineRoot/src/audio","-I$vehicleEngineRoot/src/core",'-include',"$vehicleEngineBuild/$vehicleEngineShape.h")
   foreach($vehicleEngineUnit in @('AudioVehicleEngine','AudioMath','AudioRequests','AudioSoundQueue')) {
    & $vehicleEngineCompiler -std=c11 -pedantic-errors "-$vehicleEngineOptimization" @vehicleEngineArgs -c "$vehicleEngineRoot/src/audio/$vehicleEngineUnit.c" -o "$vehicleEngineBuild/$vehicleEngineUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $vehicleEngineUnit"}
   }
   & $vehicleEngineCompiler -std=c11 -pedantic-errors "-$vehicleEngineOptimization" @vehicleEngineArgs -c "$vehicleEngineBuild/caller.c" -o "$vehicleEngineBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $vehicleEngineCommon=@("$vehicleEngineBuild/AudioMath.o","$vehicleEngineBuild/AudioRequests.o","$vehicleEngineBuild/AudioSoundQueue.o")
   & $vehicleEngineCpp -std=c++17 "-$vehicleEngineOptimization" @vehicleEngineArgs "$vehicleEngineBuild/before.cpp" @vehicleEngineCommon -o "$vehicleEngineBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $vehicleEngineCpp -std=c++17 "-$vehicleEngineOptimization" @vehicleEngineArgs "$vehicleEngineBuild/after.cpp" @vehicleEngineCommon "$vehicleEngineBuild/AudioVehicleEngine.o" "$vehicleEngineBuild/caller.o" -o "$vehicleEngineBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $vehicleEngineBefore=& "$vehicleEngineBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $vehicleEngineAfter=& "$vehicleEngineBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $vehicleEngineBefore -ne $vehicleEngineAfter) {throw "VehicleEngine comparison failed: $vehicleEngineBefore / $vehicleEngineAfter"}
   $vehicleEngineRuns += "$vehicleEngineCompiler-$vehicleEngineOptimization-$vehicleEngineShape : $vehicleEngineAfter"
   Write-Output $vehicleEngineRuns[-1]
  }
 }
}
$vehicleEngineRuns | ConvertTo-Json | Set-Content "$vehicleEngineBuild/runtime-runs.json"
$vehicleEngineSanBuild=Join-Path $vehicleEngineBuild 'sanitized'
New-Item -ItemType Directory -Path $vehicleEngineSanBuild -Force | Out-Null
$vehicleEngineSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$vehicleEngineRoot/src/audio","-I$vehicleEngineRoot/src/core",'-include',"$vehicleEngineBuild/default.h")
foreach($vehicleEngineSanUnit in @('AudioVehicleEngine','AudioMath','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @vehicleEngineSanArgs -c "$vehicleEngineRoot/src/audio/$vehicleEngineSanUnit.c" -o "$vehicleEngineSanBuild/$vehicleEngineSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $vehicleEngineSanUnit"}
}
& clang -std=c11 @vehicleEngineSanArgs -c "$vehicleEngineBuild/caller.c" -o "$vehicleEngineSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$vehicleEngineSanCommon=@("$vehicleEngineSanBuild/AudioMath.o","$vehicleEngineSanBuild/AudioRequests.o","$vehicleEngineSanBuild/AudioSoundQueue.o")
$vehicleEngineSanResults=@()
foreach($vehicleEngineSanVariant in @('before','after')) {
 $vehicleEngineSanExtra=if($vehicleEngineSanVariant -eq 'after') {@("$vehicleEngineSanBuild/AudioVehicleEngine.o","$vehicleEngineSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @vehicleEngineSanArgs "$vehicleEngineBuild/$vehicleEngineSanVariant.cpp" @vehicleEngineSanCommon @vehicleEngineSanExtra -o "$vehicleEngineSanBuild/$vehicleEngineSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $vehicleEngineSanResults += & "$vehicleEngineSanBuild/$vehicleEngineSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $vehicleEngineSanVariant"}
}
if($vehicleEngineSanResults.Count -ne 2 -or $vehicleEngineSanResults[0] -ne $vehicleEngineSanResults[1]) {throw 'Sanitized comparison failed'}
$vehicleEngineSanResults | ConvertTo-Json | Set-Content "$vehicleEngineBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($vehicleEngineSanResults[1])"
