$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$bridgeRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$bridgeBuild=Join-Path $bridgeRoot 'build/audio-bridge-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_bridge_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Bridge source audit failed'}
$bridgeRuns=@()
foreach($bridgeCompiler in @('clang','gcc')) {
 foreach($bridgeOptimization in @('O0','O2')) {
  $bridgeCpp=if($bridgeCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($bridgeShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale','no-reflections','disabled')) {
   $bridgeArgs=@("-I$bridgeRoot/src/audio","-I$bridgeRoot/src/core",'-include',"$bridgeBuild/$bridgeShape.h")
   foreach($bridgeUnit in @('AudioBridge','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
    & $bridgeCompiler -std=c11 -pedantic-errors "-$bridgeOptimization" @bridgeArgs -c "$bridgeRoot/src/audio/$bridgeUnit.c" $(if($bridgeUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestBridgeRealSubmit'}) -o "$bridgeBuild/$bridgeUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $bridgeUnit"}
   }
   & $bridgeCompiler -std=c11 -pedantic-errors "-$bridgeOptimization" @bridgeArgs -c "$bridgeBuild/caller.c" -o "$bridgeBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $bridgeCompiler -std=c11 -O2 -c "$bridgeBuild/fast_hash.c" -o "$bridgeBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $bridgeCommon=@("$bridgeBuild/fast_hash.o","$bridgeBuild/AudioMath.o","$bridgeBuild/AudioGeometry.o","$bridgeBuild/AudioRequests.o","$bridgeBuild/AudioSoundQueue.o")
   & $bridgeCpp -std=c++17 "-$bridgeOptimization" @bridgeArgs "$bridgeBuild/before.cpp" @bridgeCommon -o "$bridgeBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $bridgeCpp -std=c++17 "-$bridgeOptimization" @bridgeArgs "$bridgeBuild/after.cpp" @bridgeCommon "$bridgeBuild/AudioBridge.o" "$bridgeBuild/caller.o" -o "$bridgeBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $bridgeBefore=& "$bridgeBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $bridgeAfter=& "$bridgeBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $bridgeBefore -ne $bridgeAfter) {throw "Bridge comparison failed: $bridgeBefore / $bridgeAfter"}
   $bridgeRuns += "$bridgeCompiler-$bridgeOptimization-$bridgeShape : $bridgeAfter"
   Write-Output $bridgeRuns[-1]
  }
 }
}
$bridgeRuns | ConvertTo-Json | Set-Content "$bridgeBuild/runtime-runs.json"
$bridgeSanBuild=Join-Path $bridgeBuild 'sanitized'
New-Item -ItemType Directory -Path $bridgeSanBuild -Force | Out-Null
$bridgeSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$bridgeRoot/src/audio","-I$bridgeRoot/src/core",'-include',"$bridgeBuild/default.h")
foreach($bridgeSanUnit in @('AudioBridge','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @bridgeSanArgs -c "$bridgeRoot/src/audio/$bridgeSanUnit.c" $(if($bridgeSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestBridgeRealSubmit'}) -o "$bridgeSanBuild/$bridgeSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $bridgeSanUnit"}
}
& clang -std=c11 @bridgeSanArgs -c "$bridgeBuild/caller.c" -o "$bridgeSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @bridgeSanArgs -c "$bridgeBuild/fast_hash.c" -o "$bridgeSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$bridgeSanCommon=@("$bridgeSanBuild/fast_hash.o","$bridgeSanBuild/AudioMath.o","$bridgeSanBuild/AudioGeometry.o","$bridgeSanBuild/AudioRequests.o","$bridgeSanBuild/AudioSoundQueue.o")
$bridgeSanResults=@()
foreach($bridgeSanVariant in @('before','after')) {
 $bridgeSanExtra=if($bridgeSanVariant -eq 'after') {@("$bridgeSanBuild/AudioBridge.o","$bridgeSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @bridgeSanArgs "$bridgeBuild/$bridgeSanVariant.cpp" @bridgeSanCommon @bridgeSanExtra -o "$bridgeSanBuild/$bridgeSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $bridgeSanResults += & "$bridgeSanBuild/$bridgeSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $bridgeSanVariant"}
}
if($bridgeSanResults.Count -ne 2 -or $bridgeSanResults[0] -ne $bridgeSanResults[1]) {throw 'Sanitized comparison failed'}
$bridgeSanResults | ConvertTo-Json | Set-Content "$bridgeBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($bridgeSanResults[1])"
