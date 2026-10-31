$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$waterCannonRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$waterCannonBuild=Join-Path $waterCannonRoot 'build/audio-water-cannon-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_water_cannon_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'WaterCannon source audit failed'}
$waterCannonSanBuild=Join-Path $waterCannonBuild 'sanitized-ps2'
New-Item -ItemType Directory -Path $waterCannonSanBuild -Force | Out-Null
$waterCannonSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$waterCannonRoot/src/audio","-I$waterCannonRoot/src/core",'-include',"$waterCannonBuild/ps2.h")
foreach($waterCannonSanUnit in @('AudioWaterCannon','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @waterCannonSanArgs -c "$waterCannonRoot/src/audio/$waterCannonSanUnit.c" $(if($waterCannonSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestWaterCannonRealSubmit'}) -o "$waterCannonSanBuild/$waterCannonSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $waterCannonSanUnit"}
}
& clang -std=c11 @waterCannonSanArgs -c "$waterCannonBuild/caller.c" -o "$waterCannonSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @waterCannonSanArgs -c "$waterCannonBuild/fast_hash.c" -o "$waterCannonSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$waterCannonSanCommon=@("$waterCannonSanBuild/fast_hash.o","$waterCannonSanBuild/AudioMath.o","$waterCannonSanBuild/AudioGeometry.o","$waterCannonSanBuild/AudioRequests.o","$waterCannonSanBuild/AudioSoundQueue.o")
$waterCannonSanResults=@()
foreach($waterCannonSanVariant in @('before','after')) {
 $waterCannonSanExtra=if($waterCannonSanVariant -eq 'after') {@("$waterCannonSanBuild/AudioWaterCannon.o","$waterCannonSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @waterCannonSanArgs "$waterCannonBuild/$waterCannonSanVariant.cpp" @waterCannonSanCommon @waterCannonSanExtra -o "$waterCannonSanBuild/$waterCannonSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $waterCannonSanResults += & "$waterCannonSanBuild/$waterCannonSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $waterCannonSanVariant"}
}
if($waterCannonSanResults.Count -ne 2 -or $waterCannonSanResults[0] -ne $waterCannonSanResults[1]) {throw 'Sanitized comparison failed'}
$waterCannonSanResults | ConvertTo-Json | Set-Content "$waterCannonBuild/sanitizer-ps2-runs.json"
Write-Output "Sanitized comparison passed: $($waterCannonSanResults[1])"
