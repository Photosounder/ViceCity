$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$vehiclePlayerRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$vehiclePlayerBuild=Join-Path $vehiclePlayerRoot 'build/audio-vehicle-player-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_vehicle_player_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'VehiclePlayer source audit failed'}
$vehiclePlayerRuns=@()
foreach($vehiclePlayerCompiler in @('clang','gcc')) {
 foreach($vehiclePlayerOptimization in @('O0','O2')) {
  $vehiclePlayerCpp=if($vehiclePlayerCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($vehiclePlayerShape in @('default','vanilla','no-reverb','no-external','ps2')) {
   $vehiclePlayerArgs=@("-I$vehiclePlayerRoot/src/audio","-I$vehiclePlayerRoot/src/core",'-include',"$vehiclePlayerBuild/$vehiclePlayerShape.h")
   foreach($vehiclePlayerUnit in @('AudioVehiclePlayer','AudioMath','AudioRequests','AudioSoundQueue')) {
    & $vehiclePlayerCompiler -std=c11 -pedantic-errors "-$vehiclePlayerOptimization" @vehiclePlayerArgs -c "$vehiclePlayerRoot/src/audio/$vehiclePlayerUnit.c" -o "$vehiclePlayerBuild/$vehiclePlayerUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $vehiclePlayerUnit"}
   }
   & $vehiclePlayerCompiler -std=c11 -pedantic-errors "-$vehiclePlayerOptimization" @vehiclePlayerArgs -c "$vehiclePlayerBuild/caller.c" -o "$vehiclePlayerBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $vehiclePlayerCommon=@("$vehiclePlayerBuild/AudioMath.o","$vehiclePlayerBuild/AudioRequests.o","$vehiclePlayerBuild/AudioSoundQueue.o")
   & $vehiclePlayerCpp -std=c++17 "-$vehiclePlayerOptimization" @vehiclePlayerArgs "$vehiclePlayerBuild/before.cpp" @vehiclePlayerCommon -o "$vehiclePlayerBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $vehiclePlayerCpp -std=c++17 "-$vehiclePlayerOptimization" @vehiclePlayerArgs "$vehiclePlayerBuild/after.cpp" @vehiclePlayerCommon "$vehiclePlayerBuild/AudioVehiclePlayer.o" "$vehiclePlayerBuild/caller.o" -o "$vehiclePlayerBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $vehiclePlayerBefore=& "$vehiclePlayerBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $vehiclePlayerAfter=& "$vehiclePlayerBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $vehiclePlayerBefore -ne $vehiclePlayerAfter) {throw "VehiclePlayer comparison failed: $vehiclePlayerBefore / $vehiclePlayerAfter"}
   $vehiclePlayerRuns += "$vehiclePlayerCompiler-$vehiclePlayerOptimization-$vehiclePlayerShape : $vehiclePlayerAfter"
   Write-Output $vehiclePlayerRuns[-1]
  }
 }
}
$vehiclePlayerRuns | ConvertTo-Json | Set-Content "$vehiclePlayerBuild/runtime-runs.json"
$vehiclePlayerSanBuild=Join-Path $vehiclePlayerBuild 'sanitized'
New-Item -ItemType Directory -Path $vehiclePlayerSanBuild -Force | Out-Null
$vehiclePlayerSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$vehiclePlayerRoot/src/audio","-I$vehiclePlayerRoot/src/core",'-include',"$vehiclePlayerBuild/default.h")
foreach($vehiclePlayerSanUnit in @('AudioVehiclePlayer','AudioMath','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @vehiclePlayerSanArgs -c "$vehiclePlayerRoot/src/audio/$vehiclePlayerSanUnit.c" -o "$vehiclePlayerSanBuild/$vehiclePlayerSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $vehiclePlayerSanUnit"}
}
& clang -std=c11 @vehiclePlayerSanArgs -c "$vehiclePlayerBuild/caller.c" -o "$vehiclePlayerSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$vehiclePlayerSanCommon=@("$vehiclePlayerSanBuild/AudioMath.o","$vehiclePlayerSanBuild/AudioRequests.o","$vehiclePlayerSanBuild/AudioSoundQueue.o")
$vehiclePlayerSanResults=@()
foreach($vehiclePlayerSanVariant in @('before','after')) {
 $vehiclePlayerSanExtra=if($vehiclePlayerSanVariant -eq 'after') {@("$vehiclePlayerSanBuild/AudioVehiclePlayer.o","$vehiclePlayerSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @vehiclePlayerSanArgs "$vehiclePlayerBuild/$vehiclePlayerSanVariant.cpp" @vehiclePlayerSanCommon @vehiclePlayerSanExtra -o "$vehiclePlayerSanBuild/$vehiclePlayerSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $vehiclePlayerSanResults += & "$vehiclePlayerSanBuild/$vehiclePlayerSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $vehiclePlayerSanVariant"}
}
if($vehiclePlayerSanResults.Count -ne 2 -or $vehiclePlayerSanResults[0] -ne $vehiclePlayerSanResults[1]) {throw 'Sanitized comparison failed'}
$vehiclePlayerSanResults | ConvertTo-Json | Set-Content "$vehiclePlayerBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($vehiclePlayerSanResults[1])"
