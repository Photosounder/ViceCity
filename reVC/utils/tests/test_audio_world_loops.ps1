$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$worldLoopsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$worldLoopsBuild=Join-Path $worldLoopsRoot 'build/audio-world-loops-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_world_loops_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'WorldLoops source audit failed'}
$worldLoopsRuns=@()
foreach($worldLoopsCompiler in @('clang','gcc')) {
 foreach($worldLoopsOptimization in @('O0','O2')) {
  $worldLoopsCpp=if($worldLoopsCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($worldLoopsShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale','no-reflections')) {
   $worldLoopsArgs=@("-I$worldLoopsRoot/src/audio","-I$worldLoopsRoot/src/core",'-include',"$worldLoopsBuild/$worldLoopsShape.h")
   foreach($worldLoopsUnit in @('AudioWorldLoops','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
    & $worldLoopsCompiler -std=c11 -pedantic-errors "-$worldLoopsOptimization" @worldLoopsArgs -c "$worldLoopsRoot/src/audio/$worldLoopsUnit.c" $(if($worldLoopsUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestWorldLoopsRealSubmit'}) -o "$worldLoopsBuild/$worldLoopsUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $worldLoopsUnit"}
   }
   & $worldLoopsCompiler -std=c11 -pedantic-errors "-$worldLoopsOptimization" @worldLoopsArgs -c "$worldLoopsBuild/caller.c" -o "$worldLoopsBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $worldLoopsCompiler -std=c11 -O2 -c "$worldLoopsBuild/fast_hash.c" -o "$worldLoopsBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $worldLoopsCommon=@("$worldLoopsBuild/fast_hash.o","$worldLoopsBuild/AudioMath.o","$worldLoopsBuild/AudioGeometry.o","$worldLoopsBuild/AudioRequests.o","$worldLoopsBuild/AudioSoundQueue.o")
   & $worldLoopsCpp -std=c++17 "-$worldLoopsOptimization" @worldLoopsArgs "$worldLoopsBuild/before.cpp" @worldLoopsCommon -o "$worldLoopsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $worldLoopsCpp -std=c++17 "-$worldLoopsOptimization" @worldLoopsArgs "$worldLoopsBuild/after.cpp" @worldLoopsCommon "$worldLoopsBuild/AudioWorldLoops.o" "$worldLoopsBuild/caller.o" -o "$worldLoopsBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $worldLoopsBefore=& "$worldLoopsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $worldLoopsAfter=& "$worldLoopsBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $worldLoopsBefore -ne $worldLoopsAfter) {throw "WorldLoops comparison failed: $worldLoopsBefore / $worldLoopsAfter"}
   $worldLoopsRuns += "$worldLoopsCompiler-$worldLoopsOptimization-$worldLoopsShape : $worldLoopsAfter"
   Write-Output $worldLoopsRuns[-1]
  }
 }
}
$worldLoopsRuns | ConvertTo-Json | Set-Content "$worldLoopsBuild/runtime-runs.json"
$worldLoopsSanBuild=Join-Path $worldLoopsBuild 'sanitized'
New-Item -ItemType Directory -Path $worldLoopsSanBuild -Force | Out-Null
$worldLoopsSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$worldLoopsRoot/src/audio","-I$worldLoopsRoot/src/core",'-include',"$worldLoopsBuild/default.h")
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
$worldLoopsSanResults | ConvertTo-Json | Set-Content "$worldLoopsBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($worldLoopsSanResults[1])"
