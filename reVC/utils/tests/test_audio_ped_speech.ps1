$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$pedSpeechRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$pedSpeechBuild=Join-Path $pedSpeechRoot 'build/audio-ped-speech-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_ped_speech_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'PedSpeech source audit failed'}
$pedSpeechRuns=@()
foreach($pedSpeechCompiler in @('clang','gcc')) {
 foreach($pedSpeechOptimization in @('O0','O2')) {
  $pedSpeechCpp=if($pedSpeechCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($pedSpeechShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale')) {
   $pedSpeechArgs=@("-I$pedSpeechRoot/src/audio","-I$pedSpeechRoot/src/core",'-include',"$pedSpeechBuild/$pedSpeechShape.h")
   foreach($pedSpeechUnit in @('AudioPedSpeech','AudioMath','AudioPedComments')) {
    & $pedSpeechCompiler -std=c11 -pedantic-errors "-$pedSpeechOptimization" @pedSpeechArgs -c "$pedSpeechRoot/src/audio/$pedSpeechUnit.c" $(if($pedSpeechUnit -eq 'AudioPedComments'){'-DPedComments_Add=TestPedSpeechRealAdd'}) -o "$pedSpeechBuild/$pedSpeechUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $pedSpeechUnit"}
   }
   & $pedSpeechCompiler -std=c11 -pedantic-errors "-$pedSpeechOptimization" @pedSpeechArgs -c "$pedSpeechBuild/caller.c" -o "$pedSpeechBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $pedSpeechCompiler -std=c11 -O2 -c "$pedSpeechBuild/fast_hash.c" -o "$pedSpeechBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $pedSpeechCommon=@("$pedSpeechBuild/fast_hash.o","$pedSpeechBuild/AudioMath.o","$pedSpeechBuild/AudioPedComments.o")
   & $pedSpeechCpp -std=c++17 "-$pedSpeechOptimization" @pedSpeechArgs "$pedSpeechBuild/before.cpp" @pedSpeechCommon -o "$pedSpeechBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $pedSpeechCpp -std=c++17 "-$pedSpeechOptimization" @pedSpeechArgs "$pedSpeechBuild/after.cpp" @pedSpeechCommon "$pedSpeechBuild/AudioPedSpeech.o" "$pedSpeechBuild/caller.o" -o "$pedSpeechBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $pedSpeechBefore=& "$pedSpeechBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $pedSpeechAfter=& "$pedSpeechBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $pedSpeechBefore -ne $pedSpeechAfter) {throw "PedSpeech comparison failed: $pedSpeechBefore / $pedSpeechAfter"}
   $pedSpeechRuns += "$pedSpeechCompiler-$pedSpeechOptimization-$pedSpeechShape : $pedSpeechAfter"
   Write-Output $pedSpeechRuns[-1]
  }
 }
}
$pedSpeechRuns | ConvertTo-Json | Set-Content "$pedSpeechBuild/runtime-runs.json"
$pedSpeechSanBuild=Join-Path $pedSpeechBuild 'sanitized'
New-Item -ItemType Directory -Path $pedSpeechSanBuild -Force | Out-Null
$pedSpeechSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$pedSpeechRoot/src/audio","-I$pedSpeechRoot/src/core",'-include',"$pedSpeechBuild/default.h")
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
$pedSpeechSanResults | ConvertTo-Json | Set-Content "$pedSpeechBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($pedSpeechSanResults[1])"
