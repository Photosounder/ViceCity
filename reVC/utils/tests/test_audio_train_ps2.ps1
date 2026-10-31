$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$trainRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$trainBuild=Join-Path $trainRoot 'build/audio-train-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$trainRoot/utils/tests/audio_train_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Train source audit failed'}
$trainSanBuild=Join-Path $trainBuild 'sanitized-ps2'
New-Item -ItemType Directory -Path $trainSanBuild -Force | Out-Null
$trainSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$trainRoot/src/audio","-I$trainRoot/src/core",'-include',"$trainBuild/ps2.h")
foreach($trainSanUnit in @('AudioTrain','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @trainSanArgs -c "$trainRoot/src/audio/$trainSanUnit.c" $(if($trainSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestTrainRealSubmit'}) -o "$trainSanBuild/$trainSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $trainSanUnit"}
}
& clang -std=c11 @trainSanArgs -c "$trainBuild/caller.c" -o "$trainSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @trainSanArgs -c "$trainBuild/fast_hash.c" -o "$trainSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$trainSanCommon=@("$trainSanBuild/fast_hash.o","$trainSanBuild/AudioMath.o","$trainSanBuild/AudioGeometry.o","$trainSanBuild/AudioRequests.o","$trainSanBuild/AudioSoundQueue.o")
$trainSanResults=@()
foreach($trainSanVariant in @('before','after')) {
 $trainSanExtra=if($trainSanVariant -eq 'after') {@("$trainSanBuild/AudioTrain.o","$trainSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @trainSanArgs "$trainBuild/$trainSanVariant.cpp" @trainSanCommon @trainSanExtra -o "$trainSanBuild/$trainSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $trainSanResults += & "$trainSanBuild/$trainSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $trainSanVariant"}
}
if($trainSanResults.Count -ne 2 -or $trainSanResults[0] -ne $trainSanResults[1]) {throw 'Sanitized comparison failed'}
$trainSanResults | ConvertTo-Json | Set-Content "$trainBuild/sanitizer-ps2-runs.json"
Write-Output "Sanitized comparison passed: $($trainSanResults[1])"
