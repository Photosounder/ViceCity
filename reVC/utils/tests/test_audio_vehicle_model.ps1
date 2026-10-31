$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$vehicleModelRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$vehicleModelBuild=Join-Path $vehicleModelRoot 'build/audio-vehicle-model-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_vehicle_model_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'VehicleModel source audit failed'}
$vehicleModelRuns=@()
foreach($vehicleModelCompiler in @('clang','gcc')) {
 foreach($vehicleModelOptimization in @('O0','O2')) {
  $vehicleModelCpp=if($vehicleModelCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($vehicleModelShape in @('default','vanilla','no-reverb','no-external','ps2','alternate')) {
   $vehicleModelArgs=@("-I$vehicleModelRoot/src/audio","-I$vehicleModelRoot/src/core",'-include',"$vehicleModelBuild/$vehicleModelShape.h")
   foreach($vehicleModelUnit in @('AudioVehicleModel','AudioMath','AudioRequests','AudioSoundQueue')) {
    & $vehicleModelCompiler -std=c11 -pedantic-errors "-$vehicleModelOptimization" @vehicleModelArgs -c "$vehicleModelRoot/src/audio/$vehicleModelUnit.c" -o "$vehicleModelBuild/$vehicleModelUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $vehicleModelUnit"}
   }
   & $vehicleModelCompiler -std=c11 -pedantic-errors "-$vehicleModelOptimization" @vehicleModelArgs -c "$vehicleModelBuild/caller.c" -o "$vehicleModelBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $vehicleModelCommon=@("$vehicleModelBuild/AudioMath.o","$vehicleModelBuild/AudioRequests.o","$vehicleModelBuild/AudioSoundQueue.o")
   & $vehicleModelCpp -std=c++17 "-$vehicleModelOptimization" @vehicleModelArgs "$vehicleModelBuild/before.cpp" @vehicleModelCommon -o "$vehicleModelBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $vehicleModelCpp -std=c++17 "-$vehicleModelOptimization" @vehicleModelArgs "$vehicleModelBuild/after.cpp" @vehicleModelCommon "$vehicleModelBuild/AudioVehicleModel.o" "$vehicleModelBuild/caller.o" -o "$vehicleModelBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $vehicleModelBefore=& "$vehicleModelBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $vehicleModelAfter=& "$vehicleModelBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $vehicleModelBefore -ne $vehicleModelAfter) {throw "VehicleModel comparison failed: $vehicleModelBefore / $vehicleModelAfter"}
   $vehicleModelRuns += "$vehicleModelCompiler-$vehicleModelOptimization-$vehicleModelShape : $vehicleModelAfter"
   Write-Output $vehicleModelRuns[-1]
  }
 }
}
$vehicleModelRuns | ConvertTo-Json | Set-Content "$vehicleModelBuild/runtime-runs.json"
$vehicleModelSanBuild=Join-Path $vehicleModelBuild 'sanitized'
New-Item -ItemType Directory -Path $vehicleModelSanBuild -Force | Out-Null
$vehicleModelSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$vehicleModelRoot/src/audio","-I$vehicleModelRoot/src/core",'-include',"$vehicleModelBuild/default.h")
foreach($vehicleModelSanUnit in @('AudioVehicleModel','AudioMath','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @vehicleModelSanArgs -c "$vehicleModelRoot/src/audio/$vehicleModelSanUnit.c" -o "$vehicleModelSanBuild/$vehicleModelSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $vehicleModelSanUnit"}
}
& clang -std=c11 @vehicleModelSanArgs -c "$vehicleModelBuild/caller.c" -o "$vehicleModelSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$vehicleModelSanCommon=@("$vehicleModelSanBuild/AudioMath.o","$vehicleModelSanBuild/AudioRequests.o","$vehicleModelSanBuild/AudioSoundQueue.o")
$vehicleModelSanResults=@()
foreach($vehicleModelSanVariant in @('before','after')) {
 $vehicleModelSanExtra=if($vehicleModelSanVariant -eq 'after') {@("$vehicleModelSanBuild/AudioVehicleModel.o","$vehicleModelSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @vehicleModelSanArgs "$vehicleModelBuild/$vehicleModelSanVariant.cpp" @vehicleModelSanCommon @vehicleModelSanExtra -o "$vehicleModelSanBuild/$vehicleModelSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $vehicleModelSanResults += & "$vehicleModelSanBuild/$vehicleModelSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $vehicleModelSanVariant"}
}
if($vehicleModelSanResults.Count -ne 2 -or $vehicleModelSanResults[0] -ne $vehicleModelSanResults[1]) {throw 'Sanitized comparison failed'}
$vehicleModelSanResults | ConvertTo-Json | Set-Content "$vehicleModelBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($vehicleModelSanResults[1])"
