$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$firesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$firesBuild=Join-Path $firesRoot 'build/audio-fires-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_fires_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Fire source audit failed'}
$firesSanBuild=Join-Path $firesBuild 'sanitized-ps2'
New-Item -ItemType Directory -Path $firesSanBuild -Force | Out-Null
$firesSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$firesRoot/src/audio","-I$firesRoot/src/core",'-include',"$firesBuild/ps2.h")
foreach($firesSanUnit in @('AudioFires','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @firesSanArgs -c "$firesRoot/src/audio/$firesSanUnit.c" $(if($firesSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestFiresRealSubmit'}) -o "$firesSanBuild/$firesSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $firesSanUnit"}
}
& clang -std=c11 @firesSanArgs -c "$firesBuild/caller.c" -o "$firesSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @firesSanArgs -c "$firesBuild/fast_hash.c" -o "$firesSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$firesSanCommon=@("$firesSanBuild/fast_hash.o","$firesSanBuild/AudioMath.o","$firesSanBuild/AudioGeometry.o","$firesSanBuild/AudioRequests.o","$firesSanBuild/AudioSoundQueue.o")
$firesSanResults=@()
foreach($firesSanVariant in @('before','after')) {
 $firesSanExtra=if($firesSanVariant -eq 'after') {@("$firesSanBuild/AudioFires.o","$firesSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @firesSanArgs "$firesBuild/$firesSanVariant.cpp" @firesSanCommon @firesSanExtra -o "$firesSanBuild/$firesSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $firesSanResults += & "$firesSanBuild/$firesSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $firesSanVariant"}
}
if($firesSanResults.Count -ne 2 -or $firesSanResults[0] -ne $firesSanResults[1]) {throw 'Sanitized comparison failed'}
$firesSanResults | ConvertTo-Json | Set-Content "$firesBuild/sanitizer-ps2-runs.json"
Write-Output "Sanitized comparison passed: $($firesSanResults[1])"
