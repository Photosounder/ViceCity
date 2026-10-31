$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$pedTalkRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$pedTalkBuild=Join-Path $pedTalkRoot 'build/audio-ped-talk-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_ped_talk_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Ped source audit failed'}
$pedTalkSanBuild=Join-Path $pedTalkBuild 'sanitized-ps2'
New-Item -ItemType Directory -Path $pedTalkSanBuild -Force | Out-Null
$pedTalkSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$pedTalkRoot/src/audio","-I$pedTalkRoot/src/core",'-include',"$pedTalkBuild/ps2.h")
foreach($pedTalkSanUnit in @('AudioPedTalk')) {
 & clang -std=c11 @pedTalkSanArgs -c "$pedTalkRoot/src/audio/$pedTalkSanUnit.c" $(if($pedTalkSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestPedTalkRealSubmit'}) -o "$pedTalkSanBuild/$pedTalkSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $pedTalkSanUnit"}
}
& clang -std=c11 @pedTalkSanArgs -c "$pedTalkBuild/caller.c" -o "$pedTalkSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @pedTalkSanArgs -c "$pedTalkBuild/fast_hash.c" -o "$pedTalkSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$pedTalkSanCommon=@("$pedTalkSanBuild/fast_hash.o")
$pedTalkSanResults=@()
foreach($pedTalkSanVariant in @('before','after')) {
 $pedTalkSanExtra=if($pedTalkSanVariant -eq 'after') {@("$pedTalkSanBuild/AudioPedTalk.o","$pedTalkSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @pedTalkSanArgs "$pedTalkBuild/$pedTalkSanVariant.cpp" @pedTalkSanCommon @pedTalkSanExtra -o "$pedTalkSanBuild/$pedTalkSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $pedTalkSanResults += & "$pedTalkSanBuild/$pedTalkSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $pedTalkSanVariant"}
}
if($pedTalkSanResults.Count -ne 2 -or $pedTalkSanResults[0] -ne $pedTalkSanResults[1]) {throw 'Sanitized comparison failed'}
$pedTalkSanResults | ConvertTo-Json | Set-Content "$pedTalkBuild/sanitizer-ps2-runs.json"
Write-Output "Sanitized comparison passed: $($pedTalkSanResults[1])"
