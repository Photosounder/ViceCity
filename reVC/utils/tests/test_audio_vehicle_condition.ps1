$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$vehicleConditionRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$vehicleConditionBuild=Join-Path $vehicleConditionRoot 'build/audio-vehicle-condition-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_vehicle_condition_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'VehicleCondition source audit failed'}
$vehicleConditionRuns=@()
foreach($vehicleConditionCompiler in @('clang','gcc')) {
 foreach($vehicleConditionOptimization in @('O0','O2')) {
  $vehicleConditionCpp=if($vehicleConditionCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($vehicleConditionShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale')) {
   $vehicleConditionArgs=@("-I$vehicleConditionRoot/src/audio","-I$vehicleConditionRoot/src/core",'-include',"$vehicleConditionBuild/$vehicleConditionShape.h")
   foreach($vehicleConditionUnit in @('AudioVehicleCondition','AudioMath','AudioRequests','AudioSoundQueue')) {
    & $vehicleConditionCompiler -std=c11 -pedantic-errors "-$vehicleConditionOptimization" @vehicleConditionArgs -c "$vehicleConditionRoot/src/audio/$vehicleConditionUnit.c" -o "$vehicleConditionBuild/$vehicleConditionUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $vehicleConditionUnit"}
   }
   & $vehicleConditionCompiler -std=c11 -pedantic-errors "-$vehicleConditionOptimization" @vehicleConditionArgs -c "$vehicleConditionBuild/caller.c" -o "$vehicleConditionBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $vehicleConditionCommon=@("$vehicleConditionBuild/AudioMath.o","$vehicleConditionBuild/AudioRequests.o","$vehicleConditionBuild/AudioSoundQueue.o")
   & $vehicleConditionCpp -std=c++17 "-$vehicleConditionOptimization" @vehicleConditionArgs "$vehicleConditionBuild/before.cpp" @vehicleConditionCommon -o "$vehicleConditionBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $vehicleConditionCpp -std=c++17 "-$vehicleConditionOptimization" @vehicleConditionArgs "$vehicleConditionBuild/after.cpp" @vehicleConditionCommon "$vehicleConditionBuild/AudioVehicleCondition.o" "$vehicleConditionBuild/caller.o" -o "$vehicleConditionBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $vehicleConditionBefore=& "$vehicleConditionBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $vehicleConditionAfter=& "$vehicleConditionBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $vehicleConditionBefore -ne $vehicleConditionAfter) {throw "VehicleCondition comparison failed: $vehicleConditionBefore / $vehicleConditionAfter"}
   $vehicleConditionRuns += "$vehicleConditionCompiler-$vehicleConditionOptimization-$vehicleConditionShape : $vehicleConditionAfter"
   Write-Output $vehicleConditionRuns[-1]
  }
 }
}
$vehicleConditionRuns | ConvertTo-Json | Set-Content "$vehicleConditionBuild/runtime-runs.json"
$vehicleConditionSanBuild=Join-Path $vehicleConditionBuild 'sanitized'
New-Item -ItemType Directory -Path $vehicleConditionSanBuild -Force | Out-Null
$vehicleConditionSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$vehicleConditionRoot/src/audio","-I$vehicleConditionRoot/src/core",'-include',"$vehicleConditionBuild/default.h")
foreach($vehicleConditionSanUnit in @('AudioVehicleCondition','AudioMath','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @vehicleConditionSanArgs -c "$vehicleConditionRoot/src/audio/$vehicleConditionSanUnit.c" -o "$vehicleConditionSanBuild/$vehicleConditionSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $vehicleConditionSanUnit"}
}
& clang -std=c11 @vehicleConditionSanArgs -c "$vehicleConditionBuild/caller.c" -o "$vehicleConditionSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$vehicleConditionSanCommon=@("$vehicleConditionSanBuild/AudioMath.o","$vehicleConditionSanBuild/AudioRequests.o","$vehicleConditionSanBuild/AudioSoundQueue.o")
$vehicleConditionSanResults=@()
foreach($vehicleConditionSanVariant in @('before','after')) {
 $vehicleConditionSanExtra=if($vehicleConditionSanVariant -eq 'after') {@("$vehicleConditionSanBuild/AudioVehicleCondition.o","$vehicleConditionSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @vehicleConditionSanArgs "$vehicleConditionBuild/$vehicleConditionSanVariant.cpp" @vehicleConditionSanCommon @vehicleConditionSanExtra -o "$vehicleConditionSanBuild/$vehicleConditionSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $vehicleConditionSanResults += & "$vehicleConditionSanBuild/$vehicleConditionSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $vehicleConditionSanVariant"}
}
if($vehicleConditionSanResults.Count -ne 2 -or $vehicleConditionSanResults[0] -ne $vehicleConditionSanResults[1]) {throw 'Sanitized comparison failed'}
$vehicleConditionSanResults | ConvertTo-Json | Set-Content "$vehicleConditionBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($vehicleConditionSanResults[1])"
