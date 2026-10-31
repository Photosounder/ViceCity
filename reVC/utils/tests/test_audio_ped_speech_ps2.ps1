$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$pedSpeechRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$pedSpeechBuild=Join-Path $pedSpeechRoot 'build/audio-ped-speech-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_ped_speech_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'PedSpeech source audit failed'}
$pedSpeechSanBuild=Join-Path $pedSpeechBuild 'sanitized-ps2'
New-Item -ItemType Directory -Path $pedSpeechSanBuild -Force | Out-Null
$pedSpeechSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$pedSpeechRoot/src/audio","-I$pedSpeechRoot/src/core",'-include',"$pedSpeechBuild/ps2.h")
foreach($pedSpeechSanUnit in @('AudioPedSpeech','AudioMath','AudioPedComments')) {
 & clang -std=c11 @pedSpeechSanArgs -c "$pedSpeechRoot/src/audio/$pedSpeechSanUnit.c" $(if($pedSpeechSanUnit -eq 'AudioPedComments'){'-DPedComments_Add=TestPedSpeechRealAdd'}) -o "$pedSpeechSanBuild/$pedSpeechSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $pedSpeechSanUnit"}
}
& clang -std=c11 @pedSpeechSanArgs -c "$pedSpeechBuild/caller.c" -o "$pedSpeechSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @pedSpeechSanArgs -c "$pedSpeechBuild/fast_hash.c" -o "$pedSpeechSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$pedSpeechSanCommon=@("$pedSpeechSanBuild/fast_hash.o","$pedSpeechSanBuild/AudioMath.o","$pedSpeechSanBuild/AudioPedComments.o")
$pedSpeechSanResults=@()
foreach($pedSpeechSanVariant in @('before','after')) {
 $pedSpeechSanExtra=if($pedSpeechSanVariant -eq 'after') {@("$pedSpeechSanBuild/AudioPedSpeech.o","$pedSpeechSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @pedSpeechSanArgs "$pedSpeechBuild/$pedSpeechSanVariant.cpp" @pedSpeechSanCommon @pedSpeechSanExtra -o "$pedSpeechSanBuild/$pedSpeechSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $pedSpeechSanResults += & "$pedSpeechSanBuild/$pedSpeechSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $pedSpeechSanVariant"}
}
if($pedSpeechSanResults.Count -ne 2 -or $pedSpeechSanResults[0] -ne $pedSpeechSanResults[1]) {throw 'Sanitized comparison failed'}
$pedSpeechSanResults | ConvertTo-Json | Set-Content "$pedSpeechBuild/sanitizer-ps2-runs.json"
Write-Output "Sanitized comparison passed: $($pedSpeechSanResults[1])"
