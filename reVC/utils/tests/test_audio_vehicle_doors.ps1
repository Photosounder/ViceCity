$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$vehicleDoorsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$vehicleDoorsBuild=Join-Path $vehicleDoorsRoot 'build/audio-vehicle-doors-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_vehicle_doors_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'VehicleDoors source audit failed'}
$vehicleDoorsRuns=@()
foreach($vehicleDoorsCompiler in @('clang','gcc')) {
 foreach($vehicleDoorsOptimization in @('O0','O2')) {
  $vehicleDoorsCpp=if($vehicleDoorsCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($vehicleDoorsShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale')) {
   $vehicleDoorsArgs=@("-I$vehicleDoorsRoot/src/audio","-I$vehicleDoorsRoot/src/core",'-include',"$vehicleDoorsBuild/$vehicleDoorsShape.h")
   foreach($vehicleDoorsUnit in @('AudioVehicleDoors','AudioMath','AudioRequests','AudioSoundQueue')) {
    & $vehicleDoorsCompiler -std=c11 -pedantic-errors "-$vehicleDoorsOptimization" @vehicleDoorsArgs -c "$vehicleDoorsRoot/src/audio/$vehicleDoorsUnit.c" -o "$vehicleDoorsBuild/$vehicleDoorsUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $vehicleDoorsUnit"}
   }
   & $vehicleDoorsCompiler -std=c11 -pedantic-errors "-$vehicleDoorsOptimization" @vehicleDoorsArgs -c "$vehicleDoorsBuild/caller.c" -o "$vehicleDoorsBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $vehicleDoorsCommon=@("$vehicleDoorsBuild/AudioMath.o","$vehicleDoorsBuild/AudioRequests.o","$vehicleDoorsBuild/AudioSoundQueue.o")
   & $vehicleDoorsCpp -std=c++17 "-$vehicleDoorsOptimization" @vehicleDoorsArgs "$vehicleDoorsBuild/before.cpp" @vehicleDoorsCommon -o "$vehicleDoorsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $vehicleDoorsCpp -std=c++17 "-$vehicleDoorsOptimization" @vehicleDoorsArgs "$vehicleDoorsBuild/after.cpp" @vehicleDoorsCommon "$vehicleDoorsBuild/AudioVehicleDoors.o" "$vehicleDoorsBuild/caller.o" -o "$vehicleDoorsBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $vehicleDoorsBefore=& "$vehicleDoorsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $vehicleDoorsAfter=& "$vehicleDoorsBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $vehicleDoorsBefore -ne $vehicleDoorsAfter) {throw "VehicleDoors comparison failed: $vehicleDoorsBefore / $vehicleDoorsAfter"}
   $vehicleDoorsRuns += "$vehicleDoorsCompiler-$vehicleDoorsOptimization-$vehicleDoorsShape : $vehicleDoorsAfter"
   Write-Output $vehicleDoorsRuns[-1]
  }
 }
}
$vehicleDoorsRuns | ConvertTo-Json | Set-Content "$vehicleDoorsBuild/runtime-runs.json"
$vehicleDoorsSanBuild=Join-Path $vehicleDoorsBuild 'sanitized'
New-Item -ItemType Directory -Path $vehicleDoorsSanBuild -Force | Out-Null
$vehicleDoorsSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$vehicleDoorsRoot/src/audio","-I$vehicleDoorsRoot/src/core",'-include',"$vehicleDoorsBuild/default.h")
foreach($vehicleDoorsSanUnit in @('AudioVehicleDoors','AudioMath','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @vehicleDoorsSanArgs -c "$vehicleDoorsRoot/src/audio/$vehicleDoorsSanUnit.c" -o "$vehicleDoorsSanBuild/$vehicleDoorsSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $vehicleDoorsSanUnit"}
}
& clang -std=c11 @vehicleDoorsSanArgs -c "$vehicleDoorsBuild/caller.c" -o "$vehicleDoorsSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$vehicleDoorsSanCommon=@("$vehicleDoorsSanBuild/AudioMath.o","$vehicleDoorsSanBuild/AudioRequests.o","$vehicleDoorsSanBuild/AudioSoundQueue.o")
$vehicleDoorsSanResults=@()
foreach($vehicleDoorsSanVariant in @('before','after')) {
 $vehicleDoorsSanExtra=if($vehicleDoorsSanVariant -eq 'after') {@("$vehicleDoorsSanBuild/AudioVehicleDoors.o","$vehicleDoorsSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @vehicleDoorsSanArgs "$vehicleDoorsBuild/$vehicleDoorsSanVariant.cpp" @vehicleDoorsSanCommon @vehicleDoorsSanExtra -o "$vehicleDoorsSanBuild/$vehicleDoorsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $vehicleDoorsSanResults += & "$vehicleDoorsSanBuild/$vehicleDoorsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $vehicleDoorsSanVariant"}
}
if($vehicleDoorsSanResults.Count -ne 2 -or $vehicleDoorsSanResults[0] -ne $vehicleDoorsSanResults[1]) {throw 'Sanitized comparison failed'}
$vehicleDoorsSanResults | ConvertTo-Json | Set-Content "$vehicleDoorsBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($vehicleDoorsSanResults[1])"
