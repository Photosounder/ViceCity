$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$pedPlaybackRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$pedPlaybackBuild=Join-Path $pedPlaybackRoot 'build/audio-ped-playback-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_ped_playback_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Ped source audit failed'}
$pedPlaybackRuns=@()
foreach($pedPlaybackCompiler in @('clang','gcc')) {
 foreach($pedPlaybackOptimization in @('O0','O2')) {
  $pedPlaybackCpp=if($pedPlaybackCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($pedPlaybackShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale','no-attach')) {
   $pedPlaybackArgs=@("-I$pedPlaybackRoot/src/audio","-I$pedPlaybackRoot/src/core",'-include',"$pedPlaybackBuild/$pedPlaybackShape.h")
   foreach($pedPlaybackUnit in @('AudioPedPlayback','AudioMath','AudioPedComments')) {
    & $pedPlaybackCompiler -std=c11 -pedantic-errors "-$pedPlaybackOptimization" @pedPlaybackArgs -c "$pedPlaybackRoot/src/audio/$pedPlaybackUnit.c" $(if($pedPlaybackUnit -eq 'AudioPedComments'){'-DPedComments_Add=TestPedPlaybackRealAdd';'-DPedComments_Advance=TestPedPlaybackRealAdvance'}) -o "$pedPlaybackBuild/$pedPlaybackUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $pedPlaybackUnit"}
   }
   & $pedPlaybackCompiler -std=c11 -pedantic-errors "-$pedPlaybackOptimization" @pedPlaybackArgs -c "$pedPlaybackBuild/caller.c" -o "$pedPlaybackBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $pedPlaybackCompiler -std=c11 -O2 -c "$pedPlaybackBuild/fast_hash.c" -o "$pedPlaybackBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $pedPlaybackCommon=@("$pedPlaybackBuild/fast_hash.o","$pedPlaybackBuild/AudioMath.o","$pedPlaybackBuild/AudioPedComments.o")
   & $pedPlaybackCpp -std=c++17 "-$pedPlaybackOptimization" @pedPlaybackArgs "$pedPlaybackBuild/before.cpp" @pedPlaybackCommon -o "$pedPlaybackBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $pedPlaybackCpp -std=c++17 "-$pedPlaybackOptimization" @pedPlaybackArgs "$pedPlaybackBuild/after.cpp" @pedPlaybackCommon "$pedPlaybackBuild/AudioPedPlayback.o" "$pedPlaybackBuild/caller.o" -o "$pedPlaybackBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $pedPlaybackBefore=& "$pedPlaybackBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $pedPlaybackAfter=& "$pedPlaybackBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $pedPlaybackBefore -ne $pedPlaybackAfter) {throw "Ped comparison failed: $pedPlaybackBefore / $pedPlaybackAfter"}
   $pedPlaybackRuns += "$pedPlaybackCompiler-$pedPlaybackOptimization-$pedPlaybackShape : $pedPlaybackAfter"
   Write-Output $pedPlaybackRuns[-1]
  }
 }
}
$pedPlaybackRuns | ConvertTo-Json | Set-Content "$pedPlaybackBuild/runtime-runs.json"
$pedPlaybackSanBuild=Join-Path $pedPlaybackBuild 'sanitized'
New-Item -ItemType Directory -Path $pedPlaybackSanBuild -Force | Out-Null
$pedPlaybackSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$pedPlaybackRoot/src/audio","-I$pedPlaybackRoot/src/core",'-include',"$pedPlaybackBuild/default.h")
foreach($pedPlaybackSanUnit in @('AudioPedPlayback','AudioMath','AudioPedComments')) {
 & clang -std=c11 @pedPlaybackSanArgs -c "$pedPlaybackRoot/src/audio/$pedPlaybackSanUnit.c" $(if($pedPlaybackSanUnit -eq 'AudioPedComments'){'-DPedComments_Add=TestPedPlaybackRealAdd';'-DPedComments_Advance=TestPedPlaybackRealAdvance'}) -o "$pedPlaybackSanBuild/$pedPlaybackSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $pedPlaybackSanUnit"}
}
& clang -std=c11 @pedPlaybackSanArgs -c "$pedPlaybackBuild/caller.c" -o "$pedPlaybackSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @pedPlaybackSanArgs -c "$pedPlaybackBuild/fast_hash.c" -o "$pedPlaybackSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$pedPlaybackSanCommon=@("$pedPlaybackSanBuild/fast_hash.o","$pedPlaybackSanBuild/AudioMath.o","$pedPlaybackSanBuild/AudioPedComments.o")
$pedPlaybackSanResults=@()
foreach($pedPlaybackSanVariant in @('before','after')) {
 $pedPlaybackSanExtra=if($pedPlaybackSanVariant -eq 'after') {@("$pedPlaybackSanBuild/AudioPedPlayback.o","$pedPlaybackSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @pedPlaybackSanArgs "$pedPlaybackBuild/$pedPlaybackSanVariant.cpp" @pedPlaybackSanCommon @pedPlaybackSanExtra -o "$pedPlaybackSanBuild/$pedPlaybackSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $pedPlaybackSanResults += & "$pedPlaybackSanBuild/$pedPlaybackSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $pedPlaybackSanVariant"}
}
if($pedPlaybackSanResults.Count -ne 2 -or $pedPlaybackSanResults[0] -ne $pedPlaybackSanResults[1]) {throw 'Sanitized comparison failed'}
$pedPlaybackSanResults | ConvertTo-Json | Set-Content "$pedPlaybackBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($pedPlaybackSanResults[1])"
