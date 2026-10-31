$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$pedRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$pedBuild=Join-Path $pedRoot 'build/audio-ped-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_ped_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Ped source audit failed'}
$pedSanBuild=Join-Path $pedBuild 'sanitized-ps2'
New-Item -ItemType Directory -Path $pedSanBuild -Force | Out-Null
$pedSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$pedRoot/src/audio","-I$pedRoot/src/core",'-include',"$pedBuild/ps2.h")
foreach($pedSanUnit in @('AudioPed','AudioGeometry')) {
 & clang -std=c11 @pedSanArgs -c "$pedRoot/src/audio/$pedSanUnit.c" $(if($pedSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestPedRealSubmit'}) -o "$pedSanBuild/$pedSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $pedSanUnit"}
}
& clang -std=c11 @pedSanArgs -c "$pedBuild/caller.c" -o "$pedSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @pedSanArgs -c "$pedBuild/fast_hash.c" -o "$pedSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$pedSanCommon=@("$pedSanBuild/fast_hash.o","$pedSanBuild/AudioGeometry.o")
$pedSanResults=@()
foreach($pedSanVariant in @('before','after')) {
 $pedSanExtra=if($pedSanVariant -eq 'after') {@("$pedSanBuild/AudioPed.o","$pedSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @pedSanArgs "$pedBuild/$pedSanVariant.cpp" @pedSanCommon @pedSanExtra -o "$pedSanBuild/$pedSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $pedSanResults += & "$pedSanBuild/$pedSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $pedSanVariant"}
}
if($pedSanResults.Count -ne 2 -or $pedSanResults[0] -ne $pedSanResults[1]) {throw 'Sanitized comparison failed'}
$pedSanResults | ConvertTo-Json | Set-Content "$pedBuild/sanitizer-ps2-runs.json"
Write-Output "Sanitized comparison passed: $($pedSanResults[1])"
