$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$vehicleReverseRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$vehicleReverseBuild=Join-Path $vehicleReverseRoot 'build/audio-vehicle-reverse-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_vehicle_reverse_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'VehicleReverse source audit failed'}
$vehicleReverseRuns=@()
foreach($vehicleReverseCompiler in @('clang','gcc')) {
 foreach($vehicleReverseOptimization in @('O0','O2')) {
  $vehicleReverseCpp=if($vehicleReverseCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($vehicleReverseShape in @('default','vanilla','no-reverb','no-external','ps2')) {
   $vehicleReverseArgs=@("-I$vehicleReverseRoot/src/audio","-I$vehicleReverseRoot/src/core",'-include',"$vehicleReverseBuild/$vehicleReverseShape.h")
   foreach($vehicleReverseUnit in @('AudioVehicleReverse','AudioMath','AudioRequests','AudioSoundQueue')) {
    & $vehicleReverseCompiler -std=c11 -pedantic-errors "-$vehicleReverseOptimization" @vehicleReverseArgs -c "$vehicleReverseRoot/src/audio/$vehicleReverseUnit.c" -o "$vehicleReverseBuild/$vehicleReverseUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $vehicleReverseUnit"}
   }
   & $vehicleReverseCompiler -std=c11 -pedantic-errors "-$vehicleReverseOptimization" @vehicleReverseArgs -c "$vehicleReverseBuild/caller.c" -o "$vehicleReverseBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $vehicleReverseCommon=@("$vehicleReverseBuild/AudioMath.o","$vehicleReverseBuild/AudioRequests.o","$vehicleReverseBuild/AudioSoundQueue.o")
   & $vehicleReverseCpp -std=c++17 "-$vehicleReverseOptimization" @vehicleReverseArgs "$vehicleReverseBuild/before.cpp" @vehicleReverseCommon -o "$vehicleReverseBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $vehicleReverseCpp -std=c++17 "-$vehicleReverseOptimization" @vehicleReverseArgs "$vehicleReverseBuild/after.cpp" @vehicleReverseCommon "$vehicleReverseBuild/AudioVehicleReverse.o" "$vehicleReverseBuild/caller.o" -o "$vehicleReverseBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $vehicleReverseBefore=& "$vehicleReverseBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $vehicleReverseAfter=& "$vehicleReverseBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $vehicleReverseBefore -ne $vehicleReverseAfter) {throw "VehicleReverse comparison failed: $vehicleReverseBefore / $vehicleReverseAfter"}
   $vehicleReverseRuns += "$vehicleReverseCompiler-$vehicleReverseOptimization-$vehicleReverseShape : $vehicleReverseAfter"
   Write-Output $vehicleReverseRuns[-1]
  }
 }
}
$vehicleReverseRuns | ConvertTo-Json | Set-Content "$vehicleReverseBuild/runtime-runs.json"
$vehicleReverseSanBuild=Join-Path $vehicleReverseBuild 'sanitized'
New-Item -ItemType Directory -Path $vehicleReverseSanBuild -Force | Out-Null
$vehicleReverseSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$vehicleReverseRoot/src/audio","-I$vehicleReverseRoot/src/core",'-include',"$vehicleReverseBuild/default.h")
foreach($vehicleReverseSanUnit in @('AudioVehicleReverse','AudioMath','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @vehicleReverseSanArgs -c "$vehicleReverseRoot/src/audio/$vehicleReverseSanUnit.c" -o "$vehicleReverseSanBuild/$vehicleReverseSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $vehicleReverseSanUnit"}
}
& clang -std=c11 @vehicleReverseSanArgs -c "$vehicleReverseBuild/caller.c" -o "$vehicleReverseSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$vehicleReverseSanCommon=@("$vehicleReverseSanBuild/AudioMath.o","$vehicleReverseSanBuild/AudioRequests.o","$vehicleReverseSanBuild/AudioSoundQueue.o")
$vehicleReverseSanResults=@()
foreach($vehicleReverseSanVariant in @('before','after')) {
 $vehicleReverseSanExtra=if($vehicleReverseSanVariant -eq 'after') {@("$vehicleReverseSanBuild/AudioVehicleReverse.o","$vehicleReverseSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @vehicleReverseSanArgs "$vehicleReverseBuild/$vehicleReverseSanVariant.cpp" @vehicleReverseSanCommon @vehicleReverseSanExtra -o "$vehicleReverseSanBuild/$vehicleReverseSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $vehicleReverseSanResults += & "$vehicleReverseSanBuild/$vehicleReverseSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $vehicleReverseSanVariant"}
}
if($vehicleReverseSanResults.Count -ne 2 -or $vehicleReverseSanResults[0] -ne $vehicleReverseSanResults[1]) {throw 'Sanitized comparison failed'}
$vehicleReverseSanResults | ConvertTo-Json | Set-Content "$vehicleReverseBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($vehicleReverseSanResults[1])"
