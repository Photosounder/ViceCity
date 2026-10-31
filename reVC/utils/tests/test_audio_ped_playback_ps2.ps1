$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$pedPlaybackRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$pedPlaybackBuild=Join-Path $pedPlaybackRoot 'build/audio-ped-playback-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_ped_playback_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Ped source audit failed'}
$pedPlaybackSanBuild=Join-Path $pedPlaybackBuild 'sanitized-ps2'
New-Item -ItemType Directory -Path $pedPlaybackSanBuild -Force | Out-Null
$pedPlaybackSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$pedPlaybackRoot/src/audio","-I$pedPlaybackRoot/src/core",'-include',"$pedPlaybackBuild/ps2.h")
foreach($pedPlaybackSanUnit in @('AudioPedPlayback','AudioMath','AudioPedComments')) {
 & clang -std=c11 @pedPlaybackSanArgs -c "$pedPlaybackRoot/src/audio/$pedPlaybackSanUnit.c" $(if($pedPlaybackSanUnit -eq 'AudioPedComments'){'-DPedComments_Add=TestPedPlaybackRealAdd';'-DPedComments_Advance=TestPedPlaybackRealAdvance'}) -o "$pedPlaybackSanBuild/$pedPlaybackSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $pedPlaybackSanUnit"}
}
& clang -std=c11 @pedPlaybackSanArgs -c "$pedPlaybackBuild/caller.c" -o "$pedPlaybackSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @pedPlaybackSanArgs -c "$pedPlaybackBuild/fast_hash.c" -o "$pedPlaybackSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$pedPlaybackSanCommon=@("$pedPlaybackSanBuild/fast_hash.o","$pedPlaybackSanBuild/AudioMath.o","$pedPlaybackSanBuild/AudioPedComments.o")
$pedPlaybackSanResults=@()
foreach($pedPlaybackSanVariant in @('before','after')) {
 $pedPlaybackSanExtra=if($pedPlaybackSanVariant -eq 'after') {@("$pedPlaybackSanBuild/AudioPedPlayback.o","$pedPlaybackSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @pedPlaybackSanArgs "$pedPlaybackBuild/$pedPlaybackSanVariant.cpp" @pedPlaybackSanCommon @pedPlaybackSanExtra -o "$pedPlaybackSanBuild/$pedPlaybackSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $pedPlaybackSanResults += & "$pedPlaybackSanBuild/$pedPlaybackSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $pedPlaybackSanVariant"}
}
if($pedPlaybackSanResults.Count -ne 2 -or $pedPlaybackSanResults[0] -ne $pedPlaybackSanResults[1]) {throw 'Sanitized comparison failed'}
$pedPlaybackSanResults | ConvertTo-Json | Set-Content "$pedPlaybackBuild/sanitizer-ps2-runs.json"
Write-Output "Sanitized comparison passed: $($pedPlaybackSanResults[1])"
