$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$vehicleRainRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$vehicleRainBuild=Join-Path $vehicleRainRoot 'build/audio-vehicle-rain-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_vehicle_rain_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'VehicleRain source audit failed'}
$vehicleRainRuns=@()
foreach($vehicleRainCompiler in @('clang','gcc')) {
 foreach($vehicleRainOptimization in @('O0','O2')) {
  $vehicleRainCpp=if($vehicleRainCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($vehicleRainShape in @('default','vanilla','no-reverb','no-external','ps2')) {
   $vehicleRainArgs=@("-I$vehicleRainRoot/src/audio","-I$vehicleRainRoot/src/core",'-include',"$vehicleRainBuild/$vehicleRainShape.h")
   foreach($vehicleRainUnit in @('AudioVehicleRain','AudioMath','AudioRequests','AudioSoundQueue')) {
    & $vehicleRainCompiler -std=c11 -pedantic-errors "-$vehicleRainOptimization" @vehicleRainArgs -c "$vehicleRainRoot/src/audio/$vehicleRainUnit.c" -o "$vehicleRainBuild/$vehicleRainUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $vehicleRainUnit"}
   }
   & $vehicleRainCompiler -std=c11 -pedantic-errors "-$vehicleRainOptimization" @vehicleRainArgs -c "$vehicleRainBuild/caller.c" -o "$vehicleRainBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $vehicleRainCommon=@("$vehicleRainBuild/AudioMath.o","$vehicleRainBuild/AudioRequests.o","$vehicleRainBuild/AudioSoundQueue.o")
   & $vehicleRainCpp -std=c++17 "-$vehicleRainOptimization" @vehicleRainArgs "$vehicleRainBuild/before.cpp" @vehicleRainCommon -o "$vehicleRainBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $vehicleRainCpp -std=c++17 "-$vehicleRainOptimization" @vehicleRainArgs "$vehicleRainBuild/after.cpp" @vehicleRainCommon "$vehicleRainBuild/AudioVehicleRain.o" "$vehicleRainBuild/caller.o" -o "$vehicleRainBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $vehicleRainBefore=& "$vehicleRainBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $vehicleRainAfter=& "$vehicleRainBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $vehicleRainBefore -ne $vehicleRainAfter) {throw "VehicleRain comparison failed: $vehicleRainBefore / $vehicleRainAfter"}
   $vehicleRainRuns += "$vehicleRainCompiler-$vehicleRainOptimization-$vehicleRainShape : $vehicleRainAfter"
   Write-Output $vehicleRainRuns[-1]
  }
 }
}
$vehicleRainRuns | ConvertTo-Json | Set-Content "$vehicleRainBuild/runtime-runs.json"
$vehicleRainSanBuild=Join-Path $vehicleRainBuild 'sanitized'
New-Item -ItemType Directory -Path $vehicleRainSanBuild -Force | Out-Null
$vehicleRainSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$vehicleRainRoot/src/audio","-I$vehicleRainRoot/src/core",'-include',"$vehicleRainBuild/default.h")
foreach($vehicleRainSanUnit in @('AudioVehicleRain','AudioMath','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @vehicleRainSanArgs -c "$vehicleRainRoot/src/audio/$vehicleRainSanUnit.c" -o "$vehicleRainSanBuild/$vehicleRainSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $vehicleRainSanUnit"}
}
& clang -std=c11 @vehicleRainSanArgs -c "$vehicleRainBuild/caller.c" -o "$vehicleRainSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$vehicleRainSanCommon=@("$vehicleRainSanBuild/AudioMath.o","$vehicleRainSanBuild/AudioRequests.o","$vehicleRainSanBuild/AudioSoundQueue.o")
$vehicleRainSanResults=@()
foreach($vehicleRainSanVariant in @('before','after')) {
 $vehicleRainSanExtra=if($vehicleRainSanVariant -eq 'after') {@("$vehicleRainSanBuild/AudioVehicleRain.o","$vehicleRainSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @vehicleRainSanArgs "$vehicleRainBuild/$vehicleRainSanVariant.cpp" @vehicleRainSanCommon @vehicleRainSanExtra -o "$vehicleRainSanBuild/$vehicleRainSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $vehicleRainSanResults += & "$vehicleRainSanBuild/$vehicleRainSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $vehicleRainSanVariant"}
}
if($vehicleRainSanResults.Count -ne 2 -or $vehicleRainSanResults[0] -ne $vehicleRainSanResults[1]) {throw 'Sanitized comparison failed'}
$vehicleRainSanResults | ConvertTo-Json | Set-Content "$vehicleRainBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($vehicleRainSanResults[1])"
