$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$vehicleWarningRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$vehicleWarningBuild=Join-Path $vehicleWarningRoot 'build/audio-vehicle-warning-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_vehicle_warning_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'VehicleWarning source audit failed'}
$vehicleWarningRuns=@()
foreach($vehicleWarningCompiler in @('clang','gcc')) {
 foreach($vehicleWarningOptimization in @('O0','O2')) {
  $vehicleWarningCpp=if($vehicleWarningCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($vehicleWarningShape in @('default','vanilla','no-reverb','no-external','ps2')) {
   $vehicleWarningArgs=@("-I$vehicleWarningRoot/src/audio","-I$vehicleWarningRoot/src/core",'-include',"$vehicleWarningBuild/$vehicleWarningShape.h")
   foreach($vehicleWarningUnit in @('AudioVehicleWarning','AudioMath','AudioRequests','AudioSoundQueue')) {
    & $vehicleWarningCompiler -std=c11 -pedantic-errors "-$vehicleWarningOptimization" @vehicleWarningArgs -c "$vehicleWarningRoot/src/audio/$vehicleWarningUnit.c" -o "$vehicleWarningBuild/$vehicleWarningUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $vehicleWarningUnit"}
   }
   & $vehicleWarningCompiler -std=c11 -pedantic-errors "-$vehicleWarningOptimization" @vehicleWarningArgs -c "$vehicleWarningBuild/caller.c" -o "$vehicleWarningBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $vehicleWarningCommon=@("$vehicleWarningBuild/AudioMath.o","$vehicleWarningBuild/AudioRequests.o","$vehicleWarningBuild/AudioSoundQueue.o")
   & $vehicleWarningCpp -std=c++17 "-$vehicleWarningOptimization" @vehicleWarningArgs "$vehicleWarningBuild/before.cpp" @vehicleWarningCommon -o "$vehicleWarningBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $vehicleWarningCpp -std=c++17 "-$vehicleWarningOptimization" @vehicleWarningArgs "$vehicleWarningBuild/after.cpp" @vehicleWarningCommon "$vehicleWarningBuild/AudioVehicleWarning.o" "$vehicleWarningBuild/caller.o" -o "$vehicleWarningBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $vehicleWarningBefore=& "$vehicleWarningBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $vehicleWarningAfter=& "$vehicleWarningBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $vehicleWarningBefore -ne $vehicleWarningAfter) {throw "VehicleWarning comparison failed: $vehicleWarningBefore / $vehicleWarningAfter"}
   $vehicleWarningRuns += "$vehicleWarningCompiler-$vehicleWarningOptimization-$vehicleWarningShape : $vehicleWarningAfter"
   Write-Output $vehicleWarningRuns[-1]
  }
 }
}
$vehicleWarningRuns | ConvertTo-Json | Set-Content "$vehicleWarningBuild/runtime-runs.json"
$vehicleWarningSanBuild=Join-Path $vehicleWarningBuild 'sanitized'
New-Item -ItemType Directory -Path $vehicleWarningSanBuild -Force | Out-Null
$vehicleWarningSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$vehicleWarningRoot/src/audio","-I$vehicleWarningRoot/src/core",'-include',"$vehicleWarningBuild/default.h")
foreach($vehicleWarningSanUnit in @('AudioVehicleWarning','AudioMath','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @vehicleWarningSanArgs -c "$vehicleWarningRoot/src/audio/$vehicleWarningSanUnit.c" -o "$vehicleWarningSanBuild/$vehicleWarningSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $vehicleWarningSanUnit"}
}
& clang -std=c11 @vehicleWarningSanArgs -c "$vehicleWarningBuild/caller.c" -o "$vehicleWarningSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$vehicleWarningSanCommon=@("$vehicleWarningSanBuild/AudioMath.o","$vehicleWarningSanBuild/AudioRequests.o","$vehicleWarningSanBuild/AudioSoundQueue.o")
$vehicleWarningSanResults=@()
foreach($vehicleWarningSanVariant in @('before','after')) {
 $vehicleWarningSanExtra=if($vehicleWarningSanVariant -eq 'after') {@("$vehicleWarningSanBuild/AudioVehicleWarning.o","$vehicleWarningSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @vehicleWarningSanArgs "$vehicleWarningBuild/$vehicleWarningSanVariant.cpp" @vehicleWarningSanCommon @vehicleWarningSanExtra -o "$vehicleWarningSanBuild/$vehicleWarningSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $vehicleWarningSanResults += & "$vehicleWarningSanBuild/$vehicleWarningSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $vehicleWarningSanVariant"}
}
if($vehicleWarningSanResults.Count -ne 2 -or $vehicleWarningSanResults[0] -ne $vehicleWarningSanResults[1]) {throw 'Sanitized comparison failed'}
$vehicleWarningSanResults | ConvertTo-Json | Set-Content "$vehicleWarningBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($vehicleWarningSanResults[1])"
