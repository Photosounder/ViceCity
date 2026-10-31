$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$bridgeRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$bridgeBuild=Join-Path $bridgeRoot 'build/audio-bridge-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_bridge_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Bridge source audit failed'}
$bridgeSanBuild=Join-Path $bridgeBuild 'sanitized-ps2'
New-Item -ItemType Directory -Path $bridgeSanBuild -Force | Out-Null
$bridgeSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$bridgeRoot/src/audio","-I$bridgeRoot/src/core",'-include',"$bridgeBuild/ps2.h")
foreach($bridgeSanUnit in @('AudioBridge','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @bridgeSanArgs -c "$bridgeRoot/src/audio/$bridgeSanUnit.c" $(if($bridgeSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestBridgeRealSubmit'}) -o "$bridgeSanBuild/$bridgeSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $bridgeSanUnit"}
}
& clang -std=c11 @bridgeSanArgs -c "$bridgeBuild/caller.c" -o "$bridgeSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @bridgeSanArgs -c "$bridgeBuild/fast_hash.c" -o "$bridgeSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$bridgeSanCommon=@("$bridgeSanBuild/fast_hash.o","$bridgeSanBuild/AudioMath.o","$bridgeSanBuild/AudioGeometry.o","$bridgeSanBuild/AudioRequests.o","$bridgeSanBuild/AudioSoundQueue.o")
$bridgeSanResults=@()
foreach($bridgeSanVariant in @('before','after')) {
 $bridgeSanExtra=if($bridgeSanVariant -eq 'after') {@("$bridgeSanBuild/AudioBridge.o","$bridgeSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @bridgeSanArgs "$bridgeBuild/$bridgeSanVariant.cpp" @bridgeSanCommon @bridgeSanExtra -o "$bridgeSanBuild/$bridgeSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $bridgeSanResults += & "$bridgeSanBuild/$bridgeSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $bridgeSanVariant"}
}
if($bridgeSanResults.Count -ne 2 -or $bridgeSanResults[0] -ne $bridgeSanResults[1]) {throw 'Sanitized comparison failed'}
$bridgeSanResults | ConvertTo-Json | Set-Content "$bridgeBuild/sanitizer-ps2-runs.json"
Write-Output "Sanitized comparison passed: $($bridgeSanResults[1])"
