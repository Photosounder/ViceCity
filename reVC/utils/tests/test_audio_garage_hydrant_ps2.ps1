$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$garageHydrantRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$garageHydrantBuild=Join-Path $garageHydrantRoot 'build/audio-garage-hydrant-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_garage_hydrant_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'GarageHydrant source audit failed'}
$garageHydrantSanBuild=Join-Path $garageHydrantBuild 'sanitized-ps2'
New-Item -ItemType Directory -Path $garageHydrantSanBuild -Force | Out-Null
$garageHydrantSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$garageHydrantRoot/src/audio","-I$garageHydrantRoot/src/core",'-include',"$garageHydrantBuild/ps2.h")
foreach($garageHydrantSanUnit in @('AudioGarageHydrant','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @garageHydrantSanArgs -c "$garageHydrantRoot/src/audio/$garageHydrantSanUnit.c" $(if($garageHydrantSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestGarageHydrantRealSubmit'}) -o "$garageHydrantSanBuild/$garageHydrantSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $garageHydrantSanUnit"}
}
& clang -std=c11 @garageHydrantSanArgs -c "$garageHydrantBuild/caller.c" -o "$garageHydrantSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @garageHydrantSanArgs -c "$garageHydrantBuild/fast_hash.c" -o "$garageHydrantSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$garageHydrantSanCommon=@("$garageHydrantSanBuild/fast_hash.o","$garageHydrantSanBuild/AudioMath.o","$garageHydrantSanBuild/AudioGeometry.o","$garageHydrantSanBuild/AudioRequests.o","$garageHydrantSanBuild/AudioSoundQueue.o")
$garageHydrantSanResults=@()
foreach($garageHydrantSanVariant in @('before','after')) {
 $garageHydrantSanExtra=if($garageHydrantSanVariant -eq 'after') {@("$garageHydrantSanBuild/AudioGarageHydrant.o","$garageHydrantSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @garageHydrantSanArgs "$garageHydrantBuild/$garageHydrantSanVariant.cpp" @garageHydrantSanCommon @garageHydrantSanExtra -o "$garageHydrantSanBuild/$garageHydrantSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $garageHydrantSanResults += & "$garageHydrantSanBuild/$garageHydrantSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $garageHydrantSanVariant"}
}
if($garageHydrantSanResults.Count -ne 2 -or $garageHydrantSanResults[0] -ne $garageHydrantSanResults[1]) {throw 'Sanitized comparison failed'}
$garageHydrantSanResults | ConvertTo-Json | Set-Content "$garageHydrantBuild/sanitizer-ps2-runs.json"
Write-Output "Sanitized comparison passed: $($garageHydrantSanResults[1])"
