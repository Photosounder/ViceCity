$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$vehicleSkidRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$vehicleSkidBuild=Join-Path $vehicleSkidRoot 'build/audio-vehicle-skid-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_vehicle_skid_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'VehicleSkid source audit failed'}
$vehicleSkidRuns=@()
foreach($vehicleSkidCompiler in @('clang','gcc')) {
 foreach($vehicleSkidOptimization in @('O0','O2')) {
  $vehicleSkidCpp=if($vehicleSkidCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($vehicleSkidShape in @('default','vanilla','no-reverb','no-external','ps2')) {
   $vehicleSkidArgs=@("-I$vehicleSkidRoot/src/audio","-I$vehicleSkidRoot/src/core",'-include',"$vehicleSkidBuild/$vehicleSkidShape.h")
   foreach($vehicleSkidUnit in @('AudioVehicleSkid','AudioMath','AudioRequests','AudioSoundQueue')) {
    & $vehicleSkidCompiler -std=c11 -pedantic-errors "-$vehicleSkidOptimization" @vehicleSkidArgs -c "$vehicleSkidRoot/src/audio/$vehicleSkidUnit.c" -o "$vehicleSkidBuild/$vehicleSkidUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $vehicleSkidUnit"}
   }
   & $vehicleSkidCompiler -std=c11 -pedantic-errors "-$vehicleSkidOptimization" @vehicleSkidArgs -c "$vehicleSkidBuild/caller.c" -o "$vehicleSkidBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $vehicleSkidCommon=@("$vehicleSkidBuild/AudioMath.o","$vehicleSkidBuild/AudioRequests.o","$vehicleSkidBuild/AudioSoundQueue.o")
   & $vehicleSkidCpp -std=c++17 "-$vehicleSkidOptimization" @vehicleSkidArgs "$vehicleSkidBuild/before.cpp" @vehicleSkidCommon -o "$vehicleSkidBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $vehicleSkidCpp -std=c++17 "-$vehicleSkidOptimization" @vehicleSkidArgs "$vehicleSkidBuild/after.cpp" @vehicleSkidCommon "$vehicleSkidBuild/AudioVehicleSkid.o" "$vehicleSkidBuild/caller.o" -o "$vehicleSkidBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $vehicleSkidBefore=& "$vehicleSkidBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $vehicleSkidAfter=& "$vehicleSkidBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $vehicleSkidBefore -ne $vehicleSkidAfter) {throw "VehicleSkid comparison failed: $vehicleSkidBefore / $vehicleSkidAfter"}
   $vehicleSkidRuns += "$vehicleSkidCompiler-$vehicleSkidOptimization-$vehicleSkidShape : $vehicleSkidAfter"
   Write-Output $vehicleSkidRuns[-1]
  }
 }
}
$vehicleSkidRuns | ConvertTo-Json | Set-Content "$vehicleSkidBuild/runtime-runs.json"
$vehicleSkidSanBuild=Join-Path $vehicleSkidBuild 'sanitized'
New-Item -ItemType Directory -Path $vehicleSkidSanBuild -Force | Out-Null
$vehicleSkidSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$vehicleSkidRoot/src/audio","-I$vehicleSkidRoot/src/core",'-include',"$vehicleSkidBuild/default.h")
foreach($vehicleSkidSanUnit in @('AudioVehicleSkid','AudioMath','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @vehicleSkidSanArgs -c "$vehicleSkidRoot/src/audio/$vehicleSkidSanUnit.c" -o "$vehicleSkidSanBuild/$vehicleSkidSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $vehicleSkidSanUnit"}
}
& clang -std=c11 @vehicleSkidSanArgs -c "$vehicleSkidBuild/caller.c" -o "$vehicleSkidSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$vehicleSkidSanCommon=@("$vehicleSkidSanBuild/AudioMath.o","$vehicleSkidSanBuild/AudioRequests.o","$vehicleSkidSanBuild/AudioSoundQueue.o")
$vehicleSkidSanResults=@()
foreach($vehicleSkidSanVariant in @('before','after')) {
 $vehicleSkidSanExtra=if($vehicleSkidSanVariant -eq 'after') {@("$vehicleSkidSanBuild/AudioVehicleSkid.o","$vehicleSkidSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @vehicleSkidSanArgs "$vehicleSkidBuild/$vehicleSkidSanVariant.cpp" @vehicleSkidSanCommon @vehicleSkidSanExtra -o "$vehicleSkidSanBuild/$vehicleSkidSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $vehicleSkidSanResults += & "$vehicleSkidSanBuild/$vehicleSkidSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $vehicleSkidSanVariant"}
}
if($vehicleSkidSanResults.Count -ne 2 -or $vehicleSkidSanResults[0] -ne $vehicleSkidSanResults[1]) {throw 'Sanitized comparison failed'}
$vehicleSkidSanResults | ConvertTo-Json | Set-Content "$vehicleSkidBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($vehicleSkidSanResults[1])"
