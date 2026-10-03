$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$vehicleRoadRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$vehicleRoadBuild=Join-Path $vehicleRoadRoot 'build/audio-vehicle-road-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_vehicle_road_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'VehicleRoad source audit failed'}
$vehicleRoadRuns=@()
foreach($vehicleRoadCompiler in @('clang','gcc')) {
 foreach($vehicleRoadOptimization in @('O0','O2')) {
  $vehicleRoadCpp=if($vehicleRoadCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($vehicleRoadShape in @('default','vanilla','no-reverb','no-external','ps2')) {
   $vehicleRoadArgs=@("-I$vehicleRoadRoot/src/audio","-I$vehicleRoadRoot/src/core",'-include',"$vehicleRoadBuild/$vehicleRoadShape.h")
   foreach($vehicleRoadUnit in @('AudioVehicleRoad','AudioMath','AudioRequests','AudioSoundQueue')) {
    & $vehicleRoadCompiler -std=c11 -pedantic-errors "-$vehicleRoadOptimization" @vehicleRoadArgs -c "$vehicleRoadRoot/src/audio/$vehicleRoadUnit.c" -o "$vehicleRoadBuild/$vehicleRoadUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $vehicleRoadUnit"}
   }
   & $vehicleRoadCompiler -std=c11 -pedantic-errors "-$vehicleRoadOptimization" @vehicleRoadArgs -c "$vehicleRoadBuild/caller.c" -o "$vehicleRoadBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $vehicleRoadCommon=@("$vehicleRoadBuild/AudioMath.o","$vehicleRoadBuild/AudioRequests.o","$vehicleRoadBuild/AudioSoundQueue.o")
   & $vehicleRoadCpp -std=c++17 "-$vehicleRoadOptimization" @vehicleRoadArgs "$vehicleRoadBuild/before.cpp" @vehicleRoadCommon -o "$vehicleRoadBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $vehicleRoadCpp -std=c++17 "-$vehicleRoadOptimization" @vehicleRoadArgs "$vehicleRoadBuild/after.cpp" @vehicleRoadCommon "$vehicleRoadBuild/AudioVehicleRoad.o" "$vehicleRoadBuild/caller.o" -o "$vehicleRoadBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $vehicleRoadBefore=& "$vehicleRoadBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $vehicleRoadAfter=& "$vehicleRoadBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $vehicleRoadBefore -ne $vehicleRoadAfter) {throw "VehicleRoad comparison failed: $vehicleRoadBefore / $vehicleRoadAfter"}
   $vehicleRoadRuns += "$vehicleRoadCompiler-$vehicleRoadOptimization-$vehicleRoadShape : $vehicleRoadAfter"
   Write-Output $vehicleRoadRuns[-1]
  }
 }
}
$vehicleRoadRuns | ConvertTo-Json | Set-Content "$vehicleRoadBuild/runtime-runs.json"
$vehicleRoadSanBuild=Join-Path $vehicleRoadBuild 'sanitized'
New-Item -ItemType Directory -Path $vehicleRoadSanBuild -Force | Out-Null
$vehicleRoadSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$vehicleRoadRoot/src/audio","-I$vehicleRoadRoot/src/core",'-include',"$vehicleRoadBuild/default.h")
foreach($vehicleRoadSanUnit in @('AudioVehicleRoad','AudioMath','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @vehicleRoadSanArgs -c "$vehicleRoadRoot/src/audio/$vehicleRoadSanUnit.c" -o "$vehicleRoadSanBuild/$vehicleRoadSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $vehicleRoadSanUnit"}
}
& clang -std=c11 @vehicleRoadSanArgs -c "$vehicleRoadBuild/caller.c" -o "$vehicleRoadSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$vehicleRoadSanCommon=@("$vehicleRoadSanBuild/AudioMath.o","$vehicleRoadSanBuild/AudioRequests.o","$vehicleRoadSanBuild/AudioSoundQueue.o")
$vehicleRoadSanResults=@()
foreach($vehicleRoadSanVariant in @('before','after')) {
 $vehicleRoadSanExtra=if($vehicleRoadSanVariant -eq 'after') {@("$vehicleRoadSanBuild/AudioVehicleRoad.o","$vehicleRoadSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @vehicleRoadSanArgs "$vehicleRoadBuild/$vehicleRoadSanVariant.cpp" @vehicleRoadSanCommon @vehicleRoadSanExtra -o "$vehicleRoadSanBuild/$vehicleRoadSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $vehicleRoadSanResults += & "$vehicleRoadSanBuild/$vehicleRoadSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $vehicleRoadSanVariant"}
}
if($vehicleRoadSanResults.Count -ne 2 -or $vehicleRoadSanResults[0] -ne $vehicleRoadSanResults[1]) {throw 'Sanitized comparison failed'}
$vehicleRoadSanResults | ConvertTo-Json | Set-Content "$vehicleRoadBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($vehicleRoadSanResults[1])"
