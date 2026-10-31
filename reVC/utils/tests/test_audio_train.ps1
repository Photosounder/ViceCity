$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$trainRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$trainBuild=Join-Path $trainRoot 'build/audio-train-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_train_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Train source audit failed'}
$trainRuns=@()
foreach($trainCompiler in @('clang','gcc')) {
 foreach($trainOptimization in @('O0','O2')) {
  $trainCpp=if($trainCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($trainShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale')) {
   $trainArgs=@("-I$trainRoot/src/audio","-I$trainRoot/src/core",'-include',"$trainBuild/$trainShape.h")
   foreach($trainUnit in @('AudioTrain','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
    & $trainCompiler -std=c11 -pedantic-errors "-$trainOptimization" @trainArgs -c "$trainRoot/src/audio/$trainUnit.c" $(if($trainUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestTrainRealSubmit'}) -o "$trainBuild/$trainUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $trainUnit"}
   }
   & $trainCompiler -std=c11 -pedantic-errors "-$trainOptimization" @trainArgs -c "$trainBuild/caller.c" -o "$trainBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $trainCompiler -std=c11 -O2 -c "$trainBuild/fast_hash.c" -o "$trainBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $trainCommon=@("$trainBuild/fast_hash.o","$trainBuild/AudioMath.o","$trainBuild/AudioGeometry.o","$trainBuild/AudioRequests.o","$trainBuild/AudioSoundQueue.o")
   & $trainCpp -std=c++17 "-$trainOptimization" @trainArgs "$trainBuild/before.cpp" @trainCommon -o "$trainBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $trainCpp -std=c++17 "-$trainOptimization" @trainArgs "$trainBuild/after.cpp" @trainCommon "$trainBuild/AudioTrain.o" "$trainBuild/caller.o" -o "$trainBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $trainBefore=& "$trainBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $trainAfter=& "$trainBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $trainBefore -ne $trainAfter) {throw "Train comparison failed: $trainBefore / $trainAfter"}
   $trainRuns += "$trainCompiler-$trainOptimization-$trainShape : $trainAfter"
   Write-Output $trainRuns[-1]
  }
 }
}
$trainRuns | ConvertTo-Json | Set-Content "$trainBuild/runtime-runs.json"
$trainSanBuild=Join-Path $trainBuild 'sanitized'
New-Item -ItemType Directory -Path $trainSanBuild -Force | Out-Null
$trainSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$trainRoot/src/audio","-I$trainRoot/src/core",'-include',"$trainBuild/default.h")
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
$trainSanResults | ConvertTo-Json | Set-Content "$trainBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($trainSanResults[1])"
