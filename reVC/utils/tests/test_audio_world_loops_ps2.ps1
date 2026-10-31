$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$worldLoopsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$worldLoopsBuild=Join-Path $worldLoopsRoot 'build/audio-world-loops-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_world_loops_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'WorldLoops source audit failed'}
$worldLoopsSanBuild=Join-Path $worldLoopsBuild 'sanitized-ps2'
New-Item -ItemType Directory -Path $worldLoopsSanBuild -Force | Out-Null
$worldLoopsSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$worldLoopsRoot/src/audio","-I$worldLoopsRoot/src/core",'-include',"$worldLoopsBuild/ps2.h")
foreach($worldLoopsSanUnit in @('AudioWorldLoops','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @worldLoopsSanArgs -c "$worldLoopsRoot/src/audio/$worldLoopsSanUnit.c" $(if($worldLoopsSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestWorldLoopsRealSubmit'}) -o "$worldLoopsSanBuild/$worldLoopsSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $worldLoopsSanUnit"}
}
& clang -std=c11 @worldLoopsSanArgs -c "$worldLoopsBuild/caller.c" -o "$worldLoopsSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @worldLoopsSanArgs -c "$worldLoopsBuild/fast_hash.c" -o "$worldLoopsSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$worldLoopsSanCommon=@("$worldLoopsSanBuild/fast_hash.o","$worldLoopsSanBuild/AudioMath.o","$worldLoopsSanBuild/AudioGeometry.o","$worldLoopsSanBuild/AudioRequests.o","$worldLoopsSanBuild/AudioSoundQueue.o")
$worldLoopsSanResults=@()
foreach($worldLoopsSanVariant in @('before','after')) {
 $worldLoopsSanExtra=if($worldLoopsSanVariant -eq 'after') {@("$worldLoopsSanBuild/AudioWorldLoops.o","$worldLoopsSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @worldLoopsSanArgs "$worldLoopsBuild/$worldLoopsSanVariant.cpp" @worldLoopsSanCommon @worldLoopsSanExtra -o "$worldLoopsSanBuild/$worldLoopsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $worldLoopsSanResults += & "$worldLoopsSanBuild/$worldLoopsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $worldLoopsSanVariant"}
}
if($worldLoopsSanResults.Count -ne 2 -or $worldLoopsSanResults[0] -ne $worldLoopsSanResults[1]) {throw 'Sanitized comparison failed'}
$worldLoopsSanResults | ConvertTo-Json | Set-Content "$worldLoopsBuild/sanitizer-ps2-runs.json"
Write-Output "Sanitized comparison passed: $($worldLoopsSanResults[1])"
