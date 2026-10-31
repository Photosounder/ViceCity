$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$oneShotsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$oneShotsBuild=Join-Path $oneShotsRoot 'build/audio-vehicle-one-shots-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_vehicle_one_shots_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Plane source audit failed'}
$oneShotsRuns=@()
foreach($oneShotsCompiler in @('clang','gcc')) {
 foreach($oneShotsOptimization in @('O0','O2')) {
  $oneShotsCpp=if($oneShotsCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($oneShotsShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale','train','event-reload')) {
   $oneShotsArgs=@("-I$oneShotsRoot/src/audio","-I$oneShotsRoot/src/core",'-include',"$oneShotsBuild/$oneShotsShape.h")
   foreach($oneShotsUnit in @('AudioVehicleOneShots','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
    & $oneShotsCompiler -std=c11 -pedantic-errors "-$oneShotsOptimization" @oneShotsArgs -c "$oneShotsRoot/src/audio/$oneShotsUnit.c" $(if($oneShotsUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestVehicleOneShotsRealSubmit'}) -o "$oneShotsBuild/$oneShotsUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $oneShotsUnit"}
   }
   & $oneShotsCompiler -std=c11 -pedantic-errors "-$oneShotsOptimization" @oneShotsArgs -c "$oneShotsBuild/caller.c" -o "$oneShotsBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $oneShotsCompiler -std=c11 -O2 -c "$oneShotsBuild/fast_hash.c" -o "$oneShotsBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $oneShotsCommon=@("$oneShotsBuild/fast_hash.o","$oneShotsBuild/AudioMath.o","$oneShotsBuild/AudioGeometry.o","$oneShotsBuild/AudioRequests.o","$oneShotsBuild/AudioSoundQueue.o")
   & $oneShotsCpp -std=c++17 "-$oneShotsOptimization" @oneShotsArgs "$oneShotsBuild/before.cpp" @oneShotsCommon -o "$oneShotsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $oneShotsCpp -std=c++17 "-$oneShotsOptimization" @oneShotsArgs "$oneShotsBuild/after.cpp" @oneShotsCommon "$oneShotsBuild/AudioVehicleOneShots.o" "$oneShotsBuild/caller.o" -o "$oneShotsBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $oneShotsBefore=& "$oneShotsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $oneShotsAfter=& "$oneShotsBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $oneShotsBefore -ne $oneShotsAfter) {throw "Plane comparison failed: $oneShotsBefore / $oneShotsAfter"}
   $oneShotsRuns += "$oneShotsCompiler-$oneShotsOptimization-$oneShotsShape : $oneShotsAfter"
   Write-Output $oneShotsRuns[-1]
  }
 }
}
$oneShotsRuns | ConvertTo-Json | Set-Content "$oneShotsBuild/runtime-runs.json"
$oneShotsSanBuild=Join-Path $oneShotsBuild 'sanitized'
New-Item -ItemType Directory -Path $oneShotsSanBuild -Force | Out-Null
$oneShotsSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$oneShotsRoot/src/audio","-I$oneShotsRoot/src/core",'-include',"$oneShotsBuild/default.h")
foreach($oneShotsSanUnit in @('AudioVehicleOneShots','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @oneShotsSanArgs -c "$oneShotsRoot/src/audio/$oneShotsSanUnit.c" $(if($oneShotsSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestVehicleOneShotsRealSubmit'}) -o "$oneShotsSanBuild/$oneShotsSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $oneShotsSanUnit"}
}
& clang -std=c11 @oneShotsSanArgs -c "$oneShotsBuild/caller.c" -o "$oneShotsSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @oneShotsSanArgs -c "$oneShotsBuild/fast_hash.c" -o "$oneShotsSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$oneShotsSanCommon=@("$oneShotsSanBuild/fast_hash.o","$oneShotsSanBuild/AudioMath.o","$oneShotsSanBuild/AudioGeometry.o","$oneShotsSanBuild/AudioRequests.o","$oneShotsSanBuild/AudioSoundQueue.o")
$oneShotsSanResults=@()
foreach($oneShotsSanVariant in @('before','after')) {
 $oneShotsSanExtra=if($oneShotsSanVariant -eq 'after') {@("$oneShotsSanBuild/AudioVehicleOneShots.o","$oneShotsSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @oneShotsSanArgs "$oneShotsBuild/$oneShotsSanVariant.cpp" @oneShotsSanCommon @oneShotsSanExtra -o "$oneShotsSanBuild/$oneShotsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $oneShotsSanResults += & "$oneShotsSanBuild/$oneShotsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $oneShotsSanVariant"}
}
if($oneShotsSanResults.Count -ne 2 -or $oneShotsSanResults[0] -ne $oneShotsSanResults[1]) {throw 'Sanitized comparison failed'}
$oneShotsSanResults | ConvertTo-Json | Set-Content "$oneShotsBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($oneShotsSanResults[1])"
