$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$gameSetupRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$gameSetupBuild=Join-Path $gameSetupRoot 'build/audio-game-setup-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_game_setup_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'GameSetup source audit failed'}
$gameSetupSanBuild=Join-Path $gameSetupBuild 'sanitized-ps2'
New-Item -ItemType Directory -Path $gameSetupSanBuild -Force | Out-Null
$gameSetupSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$gameSetupRoot/src/audio","-I$gameSetupRoot/src/core",'-include',"$gameSetupBuild/ps2.h")
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
$gameSetupSanResults | ConvertTo-Json | Set-Content "$gameSetupBuild/sanitizer-ps2-runs.json"
Write-Output "Sanitized comparison passed: $($gameSetupSanResults[1])"
