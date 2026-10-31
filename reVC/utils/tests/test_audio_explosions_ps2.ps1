$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$explosionsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$explosionsBuild=Join-Path $explosionsRoot 'build/audio-explosions-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_explosions_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Explosion source audit failed'}
$explosionsSanBuild=Join-Path $explosionsBuild 'sanitized-ps2'
New-Item -ItemType Directory -Path $explosionsSanBuild -Force | Out-Null
$explosionsSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$explosionsRoot/src/audio","-I$explosionsRoot/src/core",'-include',"$explosionsBuild/ps2.h")
foreach($explosionsSanUnit in @('AudioExplosions','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @explosionsSanArgs -c "$explosionsRoot/src/audio/$explosionsSanUnit.c" $(if($explosionsSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestExplosionsRealSubmit'}) -o "$explosionsSanBuild/$explosionsSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $explosionsSanUnit"}
}
& clang -std=c11 @explosionsSanArgs -c "$explosionsBuild/caller.c" -o "$explosionsSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @explosionsSanArgs -c "$explosionsBuild/fast_hash.c" -o "$explosionsSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$explosionsSanCommon=@("$explosionsSanBuild/fast_hash.o","$explosionsSanBuild/AudioMath.o","$explosionsSanBuild/AudioGeometry.o","$explosionsSanBuild/AudioRequests.o","$explosionsSanBuild/AudioSoundQueue.o")
$explosionsSanResults=@()
foreach($explosionsSanVariant in @('before','after')) {
 $explosionsSanExtra=if($explosionsSanVariant -eq 'after') {@("$explosionsSanBuild/AudioExplosions.o","$explosionsSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @explosionsSanArgs "$explosionsBuild/$explosionsSanVariant.cpp" @explosionsSanCommon @explosionsSanExtra -o "$explosionsSanBuild/$explosionsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $explosionsSanResults += & "$explosionsSanBuild/$explosionsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $explosionsSanVariant"}
}
if($explosionsSanResults.Count -ne 2 -or $explosionsSanResults[0] -ne $explosionsSanResults[1]) {throw 'Sanitized comparison failed'}
$explosionsSanResults | ConvertTo-Json | Set-Content "$explosionsBuild/sanitizer-ps2-runs.json"
Write-Output "Sanitized comparison passed: $($explosionsSanResults[1])"
