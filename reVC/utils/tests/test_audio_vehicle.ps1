$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$vehicleRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$vehicleBuild=Join-Path $vehicleRoot 'build/audio-vehicle-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_vehicle_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Vehicle source audit failed'}
$vehicleRuns=@()
foreach($vehicleCompiler in @('clang','gcc')) {
 foreach($vehicleOptimization in @('O0','O2')) {
  $vehicleCpp=if($vehicleCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($vehicleShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale')) {
   $vehicleArgs=@("-I$vehicleRoot/src/audio","-I$vehicleRoot/src/core",'-include',"$vehicleBuild/$vehicleShape.h")
   foreach($vehicleUnit in @('AudioVehicle','AudioMath','AudioGeometry')) {
    & $vehicleCompiler -std=c11 -pedantic-errors "-$vehicleOptimization" @vehicleArgs -c "$vehicleRoot/src/audio/$vehicleUnit.c" -o "$vehicleBuild/$vehicleUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $vehicleUnit"}
   }
   & $vehicleCompiler -std=c11 -pedantic-errors "-$vehicleOptimization" @vehicleArgs -c "$vehicleBuild/caller.c" -o "$vehicleBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $vehicleCompiler -std=c11 -O2 -c "$vehicleBuild/fast_hash.c" -o "$vehicleBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $vehicleCommon=@("$vehicleBuild/fast_hash.o","$vehicleBuild/AudioMath.o","$vehicleBuild/AudioGeometry.o")
   & $vehicleCpp -std=c++17 "-$vehicleOptimization" @vehicleArgs "$vehicleBuild/before.cpp" @vehicleCommon -o "$vehicleBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $vehicleCpp -std=c++17 "-$vehicleOptimization" @vehicleArgs "$vehicleBuild/after.cpp" @vehicleCommon "$vehicleBuild/AudioVehicle.o" "$vehicleBuild/caller.o" -o "$vehicleBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $vehicleBefore=& "$vehicleBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $vehicleAfter=& "$vehicleBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $vehicleBefore -ne $vehicleAfter) {throw "Vehicle comparison failed: $vehicleBefore / $vehicleAfter"}
   $vehicleRuns += "$vehicleCompiler-$vehicleOptimization-$vehicleShape : $vehicleAfter"
   Write-Output $vehicleRuns[-1]
  }
 }
}
$vehicleRuns | ConvertTo-Json | Set-Content "$vehicleBuild/runtime-runs.json"
$vehicleSanBuild=Join-Path $vehicleBuild 'sanitized'
New-Item -ItemType Directory -Path $vehicleSanBuild -Force | Out-Null
$vehicleSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$vehicleRoot/src/audio","-I$vehicleRoot/src/core",'-include',"$vehicleBuild/default.h")
foreach($vehicleSanUnit in @('AudioVehicle','AudioMath','AudioGeometry')) {
 & clang -std=c11 @vehicleSanArgs -c "$vehicleRoot/src/audio/$vehicleSanUnit.c" -o "$vehicleSanBuild/$vehicleSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $vehicleSanUnit"}
}
& clang -std=c11 @vehicleSanArgs -c "$vehicleBuild/caller.c" -o "$vehicleSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @vehicleSanArgs -c "$vehicleBuild/fast_hash.c" -o "$vehicleSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$vehicleSanCommon=@("$vehicleSanBuild/fast_hash.o","$vehicleSanBuild/AudioMath.o","$vehicleSanBuild/AudioGeometry.o")
$vehicleSanResults=@()
foreach($vehicleSanVariant in @('before','after')) {
 $vehicleSanExtra=if($vehicleSanVariant -eq 'after') {@("$vehicleSanBuild/AudioVehicle.o","$vehicleSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @vehicleSanArgs "$vehicleBuild/$vehicleSanVariant.cpp" @vehicleSanCommon @vehicleSanExtra -o "$vehicleSanBuild/$vehicleSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $vehicleSanResults += & "$vehicleSanBuild/$vehicleSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $vehicleSanVariant"}
}
if($vehicleSanResults.Count -ne 2 -or $vehicleSanResults[0] -ne $vehicleSanResults[1]) {throw 'Sanitized comparison failed'}
$vehicleSanResults | ConvertTo-Json | Set-Content "$vehicleBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($vehicleSanResults[1])"
