$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$playerEngineRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$playerEngineBuild=Join-Path $playerEngineRoot 'build/audio-player-engine-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_player_engine_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'PlayerEngine source audit failed'}
$playerEngineRuns=@()
foreach($playerEngineCompiler in @('clang','gcc')) {
 foreach($playerEngineOptimization in @('O0','O2')) {
  $playerEngineCpp=if($playerEngineCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($playerEngineShape in @('default','vanilla','no-reverb','no-external','ps2','no-time-scale')) {
   $playerEngineArgs=@("-I$playerEngineRoot/src/audio","-I$playerEngineRoot/src/core",'-include',"$playerEngineBuild/$playerEngineShape.h")
   foreach($playerEngineUnit in @('AudioPlayerEngine','AudioMath','AudioRequests','AudioSoundQueue','AudioGeometry','AudioVehiclePlayer')) {
    & $playerEngineCompiler -std=c11 -pedantic-errors "-$playerEngineOptimization" @playerEngineArgs -c "$playerEngineRoot/src/audio/$playerEngineUnit.c" -o "$playerEngineBuild/$playerEngineUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $playerEngineUnit"}
   }
   & $playerEngineCompiler -std=c11 -pedantic-errors "-$playerEngineOptimization" @playerEngineArgs -c "$playerEngineBuild/caller.c" -o "$playerEngineBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $playerEngineCommon=@("$playerEngineBuild/AudioMath.o","$playerEngineBuild/AudioRequests.o","$playerEngineBuild/AudioSoundQueue.o","$playerEngineBuild/AudioGeometry.o","$playerEngineBuild/AudioVehiclePlayer.o")
   & $playerEngineCpp -std=c++17 "-$playerEngineOptimization" @playerEngineArgs "$playerEngineBuild/before.cpp" @playerEngineCommon -o "$playerEngineBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $playerEngineCpp -std=c++17 "-$playerEngineOptimization" @playerEngineArgs "$playerEngineBuild/after.cpp" @playerEngineCommon "$playerEngineBuild/AudioPlayerEngine.o" "$playerEngineBuild/caller.o" -o "$playerEngineBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $playerEngineBefore=& "$playerEngineBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $playerEngineAfter=& "$playerEngineBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $playerEngineBefore -ne $playerEngineAfter) {throw "PlayerEngine comparison failed: $playerEngineBefore / $playerEngineAfter"}
   $playerEngineRuns += "$playerEngineCompiler-$playerEngineOptimization-$playerEngineShape : $playerEngineAfter"
   Write-Output $playerEngineRuns[-1]
  }
 }
}
$playerEngineRuns | ConvertTo-Json | Set-Content "$playerEngineBuild/runtime-runs.json"
$playerEngineSanBuild=Join-Path $playerEngineBuild 'sanitized'
New-Item -ItemType Directory -Path $playerEngineSanBuild -Force | Out-Null
$playerEngineSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$playerEngineRoot/src/audio","-I$playerEngineRoot/src/core",'-include',"$playerEngineBuild/default.h")
foreach($playerEngineSanUnit in @('AudioPlayerEngine','AudioMath','AudioRequests','AudioSoundQueue','AudioGeometry','AudioVehiclePlayer')) {
 & clang -std=c11 @playerEngineSanArgs -c "$playerEngineRoot/src/audio/$playerEngineSanUnit.c" -o "$playerEngineSanBuild/$playerEngineSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $playerEngineSanUnit"}
}
& clang -std=c11 @playerEngineSanArgs -c "$playerEngineBuild/caller.c" -o "$playerEngineSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$playerEngineSanCommon=@("$playerEngineSanBuild/AudioMath.o","$playerEngineSanBuild/AudioRequests.o","$playerEngineSanBuild/AudioSoundQueue.o","$playerEngineSanBuild/AudioGeometry.o","$playerEngineSanBuild/AudioVehiclePlayer.o")
$playerEngineSanResults=@()
foreach($playerEngineSanVariant in @('before','after')) {
 $playerEngineSanExtra=if($playerEngineSanVariant -eq 'after') {@("$playerEngineSanBuild/AudioPlayerEngine.o","$playerEngineSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @playerEngineSanArgs "$playerEngineBuild/$playerEngineSanVariant.cpp" @playerEngineSanCommon @playerEngineSanExtra -o "$playerEngineSanBuild/$playerEngineSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $playerEngineSanResults += & "$playerEngineSanBuild/$playerEngineSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $playerEngineSanVariant"}
}
if($playerEngineSanResults.Count -ne 2 -or $playerEngineSanResults[0] -ne $playerEngineSanResults[1]) {throw 'Sanitized comparison failed'}
$playerEngineSanResults | ConvertTo-Json | Set-Content "$playerEngineBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($playerEngineSanResults[1])"
