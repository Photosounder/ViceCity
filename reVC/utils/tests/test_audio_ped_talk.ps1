$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$pedTalkRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$pedTalkBuild=Join-Path $pedTalkRoot 'build/audio-ped-talk-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_ped_talk_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Ped source audit failed'}
$pedTalkRuns=@()
foreach($pedTalkCompiler in @('clang','gcc')) {
 foreach($pedTalkOptimization in @('O0','O2')) {
  $pedTalkCpp=if($pedTalkCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($pedTalkShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale')) {
   $pedTalkArgs=@("-I$pedTalkRoot/src/audio","-I$pedTalkRoot/src/core",'-include',"$pedTalkBuild/$pedTalkShape.h")
   foreach($pedTalkUnit in @('AudioPedTalk')) {
    & $pedTalkCompiler -std=c11 -pedantic-errors "-$pedTalkOptimization" @pedTalkArgs -c "$pedTalkRoot/src/audio/$pedTalkUnit.c" $(if($pedTalkUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestPedTalkRealSubmit'}) -o "$pedTalkBuild/$pedTalkUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $pedTalkUnit"}
   }
   & $pedTalkCompiler -std=c11 -pedantic-errors "-$pedTalkOptimization" @pedTalkArgs -c "$pedTalkBuild/caller.c" -o "$pedTalkBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $pedTalkCompiler -std=c11 -O2 -c "$pedTalkBuild/fast_hash.c" -o "$pedTalkBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $pedTalkCommon=@("$pedTalkBuild/fast_hash.o")
   & $pedTalkCpp -std=c++17 "-$pedTalkOptimization" @pedTalkArgs "$pedTalkBuild/before.cpp" @pedTalkCommon -o "$pedTalkBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $pedTalkCpp -std=c++17 "-$pedTalkOptimization" @pedTalkArgs "$pedTalkBuild/after.cpp" @pedTalkCommon "$pedTalkBuild/AudioPedTalk.o" "$pedTalkBuild/caller.o" -o "$pedTalkBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $pedTalkBefore=& "$pedTalkBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $pedTalkAfter=& "$pedTalkBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $pedTalkBefore -ne $pedTalkAfter) {throw "Ped comparison failed: $pedTalkBefore / $pedTalkAfter"}
   $pedTalkRuns += "$pedTalkCompiler-$pedTalkOptimization-$pedTalkShape : $pedTalkAfter"
   Write-Output $pedTalkRuns[-1]
  }
 }
}
$pedTalkRuns | ConvertTo-Json | Set-Content "$pedTalkBuild/runtime-runs.json"
$pedTalkSanBuild=Join-Path $pedTalkBuild 'sanitized'
New-Item -ItemType Directory -Path $pedTalkSanBuild -Force | Out-Null
$pedTalkSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$pedTalkRoot/src/audio","-I$pedTalkRoot/src/core",'-include',"$pedTalkBuild/default.h")
foreach($pedTalkSanUnit in @('AudioPedTalk')) {
 & clang -std=c11 @pedTalkSanArgs -c "$pedTalkRoot/src/audio/$pedTalkSanUnit.c" $(if($pedTalkSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestPedTalkRealSubmit'}) -o "$pedTalkSanBuild/$pedTalkSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $pedTalkSanUnit"}
}
& clang -std=c11 @pedTalkSanArgs -c "$pedTalkBuild/caller.c" -o "$pedTalkSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @pedTalkSanArgs -c "$pedTalkBuild/fast_hash.c" -o "$pedTalkSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$pedTalkSanCommon=@("$pedTalkSanBuild/fast_hash.o")
$pedTalkSanResults=@()
foreach($pedTalkSanVariant in @('before','after')) {
 $pedTalkSanExtra=if($pedTalkSanVariant -eq 'after') {@("$pedTalkSanBuild/AudioPedTalk.o","$pedTalkSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @pedTalkSanArgs "$pedTalkBuild/$pedTalkSanVariant.cpp" @pedTalkSanCommon @pedTalkSanExtra -o "$pedTalkSanBuild/$pedTalkSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $pedTalkSanResults += & "$pedTalkSanBuild/$pedTalkSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $pedTalkSanVariant"}
}
if($pedTalkSanResults.Count -ne 2 -or $pedTalkSanResults[0] -ne $pedTalkSanResults[1]) {throw 'Sanitized comparison failed'}
$pedTalkSanResults | ConvertTo-Json | Set-Content "$pedTalkBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($pedTalkSanResults[1])"
