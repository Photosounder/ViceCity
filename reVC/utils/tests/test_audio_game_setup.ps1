$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$gameSetupRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$gameSetupBuild=Join-Path $gameSetupRoot 'build/audio-game-setup-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_game_setup_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'GameSetup source audit failed'}
$gameSetupRuns=@()
foreach($gameSetupCompiler in @('clang','gcc')) {
 foreach($gameSetupOptimization in @('O0','O2')) {
  $gameSetupCpp=if($gameSetupCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($gameSetupShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale','no-reflections','bridge')) {
   $gameSetupArgs=@("-I$gameSetupRoot/src/audio","-I$gameSetupRoot/src/core",'-include',"$gameSetupBuild/$gameSetupShape.h")
   foreach($gameSetupUnit in @('AudioGameSetup','AudioManagerEntities','AudioEntities','AudioService','AudioMissionState','AudioMissionPosition')) {
    & $gameSetupCompiler -std=c11 -pedantic-errors "-$gameSetupOptimization" @gameSetupArgs -c "$gameSetupRoot/src/audio/$gameSetupUnit.c" $(switch($gameSetupUnit) {'AudioManagerEntities' {'-DAudioManager_CreateEntity=TestGameSetupRealCreate'} 'AudioEntities' {'-DAudioEntities_Destroy=TestGameSetupRealDestroy';'-DAudioEntities_SetStatus=TestGameSetupRealStatus'} 'AudioService' {'-DAudioService_ResetLogicTimers=TestGameSetupRealReset'}}) -o "$gameSetupBuild/$gameSetupUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $gameSetupUnit"}
   }
   & $gameSetupCompiler -std=c11 -pedantic-errors "-$gameSetupOptimization" @gameSetupArgs -c "$gameSetupBuild/caller.c" -o "$gameSetupBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $gameSetupCompiler -std=c11 -O2 -c "$gameSetupBuild/fast_hash.c" -o "$gameSetupBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $gameSetupCommon=@("$gameSetupBuild/fast_hash.o","$gameSetupBuild/AudioManagerEntities.o","$gameSetupBuild/AudioEntities.o","$gameSetupBuild/AudioService.o","$gameSetupBuild/AudioMissionState.o","$gameSetupBuild/AudioMissionPosition.o")
   & $gameSetupCpp -std=c++17 "-$gameSetupOptimization" @gameSetupArgs "$gameSetupBuild/before.cpp" @gameSetupCommon -o "$gameSetupBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $gameSetupCpp -std=c++17 "-$gameSetupOptimization" @gameSetupArgs "$gameSetupBuild/after.cpp" @gameSetupCommon "$gameSetupBuild/AudioGameSetup.o" "$gameSetupBuild/caller.o" -o "$gameSetupBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $gameSetupBefore=& "$gameSetupBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $gameSetupAfter=& "$gameSetupBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $gameSetupBefore -ne $gameSetupAfter) {throw "GameSetup comparison failed: $gameSetupBefore / $gameSetupAfter"}
   $gameSetupRuns += "$gameSetupCompiler-$gameSetupOptimization-$gameSetupShape : $gameSetupAfter"
   Write-Output $gameSetupRuns[-1]
  }
 }
}
$gameSetupRuns | ConvertTo-Json | Set-Content "$gameSetupBuild/runtime-runs.json"
$gameSetupSanBuild=Join-Path $gameSetupBuild 'sanitized'
New-Item -ItemType Directory -Path $gameSetupSanBuild -Force | Out-Null
$gameSetupSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$gameSetupRoot/src/audio","-I$gameSetupRoot/src/core",'-include',"$gameSetupBuild/default.h")
foreach($gameSetupSanUnit in @('AudioGameSetup','AudioManagerEntities','AudioEntities','AudioService','AudioMissionState','AudioMissionPosition')) {
 & clang -std=c11 @gameSetupSanArgs -c "$gameSetupRoot/src/audio/$gameSetupSanUnit.c" $(switch($gameSetupSanUnit) {'AudioManagerEntities' {'-DAudioManager_CreateEntity=TestGameSetupRealCreate'} 'AudioEntities' {'-DAudioEntities_Destroy=TestGameSetupRealDestroy';'-DAudioEntities_SetStatus=TestGameSetupRealStatus'} 'AudioService' {'-DAudioService_ResetLogicTimers=TestGameSetupRealReset'}}) -o "$gameSetupSanBuild/$gameSetupSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $gameSetupSanUnit"}
}
& clang -std=c11 @gameSetupSanArgs -c "$gameSetupBuild/caller.c" -o "$gameSetupSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @gameSetupSanArgs -c "$gameSetupBuild/fast_hash.c" -o "$gameSetupSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$gameSetupSanCommon=@("$gameSetupSanBuild/fast_hash.o","$gameSetupSanBuild/AudioManagerEntities.o","$gameSetupSanBuild/AudioEntities.o","$gameSetupSanBuild/AudioService.o","$gameSetupSanBuild/AudioMissionState.o","$gameSetupSanBuild/AudioMissionPosition.o")
$gameSetupSanResults=@()
foreach($gameSetupSanVariant in @('before','after')) {
 $gameSetupSanExtra=if($gameSetupSanVariant -eq 'after') {@("$gameSetupSanBuild/AudioGameSetup.o","$gameSetupSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @gameSetupSanArgs "$gameSetupBuild/$gameSetupSanVariant.cpp" @gameSetupSanCommon @gameSetupSanExtra -o "$gameSetupSanBuild/$gameSetupSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $gameSetupSanResults += & "$gameSetupSanBuild/$gameSetupSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $gameSetupSanVariant"}
}
if($gameSetupSanResults.Count -ne 2 -or $gameSetupSanResults[0] -ne $gameSetupSanResults[1]) {throw 'Sanitized comparison failed'}
$gameSetupSanResults | ConvertTo-Json | Set-Content "$gameSetupBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($gameSetupSanResults[1])"
