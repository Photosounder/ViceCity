$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$pedOneShotsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$pedOneShotsBuild=Join-Path $pedOneShotsRoot 'build/audio-ped-one-shots-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_ped_one_shots_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'PedOneShots source audit failed'}
$pedOneShotsRuns=@()
foreach($pedOneShotsCompiler in @('clang','gcc')) {
 foreach($pedOneShotsOptimization in @('O0','O2')) {
  $pedOneShotsCpp=if($pedOneShotsCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($pedOneShotsShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale')) {
   $pedOneShotsArgs=@("-I$pedOneShotsRoot/src/audio","-I$pedOneShotsRoot/src/core",'-include',"$pedOneShotsBuild/$pedOneShotsShape.h")
   foreach($pedOneShotsUnit in @('AudioPedOneShots','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
    & $pedOneShotsCompiler -std=c11 -pedantic-errors "-$pedOneShotsOptimization" @pedOneShotsArgs -c "$pedOneShotsRoot/src/audio/$pedOneShotsUnit.c" $(if($pedOneShotsUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestPedOneShotsRealSubmit'}) -o "$pedOneShotsBuild/$pedOneShotsUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $pedOneShotsUnit"}
   }
   & $pedOneShotsCompiler -std=c11 -pedantic-errors "-$pedOneShotsOptimization" @pedOneShotsArgs -c "$pedOneShotsBuild/caller.c" -o "$pedOneShotsBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $pedOneShotsCompiler -std=c11 -O2 -c "$pedOneShotsBuild/fast_hash.c" -o "$pedOneShotsBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $pedOneShotsCommon=@("$pedOneShotsBuild/fast_hash.o","$pedOneShotsBuild/AudioMath.o","$pedOneShotsBuild/AudioGeometry.o","$pedOneShotsBuild/AudioRequests.o","$pedOneShotsBuild/AudioSoundQueue.o")
   & $pedOneShotsCpp -std=c++17 "-$pedOneShotsOptimization" @pedOneShotsArgs "$pedOneShotsBuild/before.cpp" @pedOneShotsCommon -o "$pedOneShotsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $pedOneShotsCpp -std=c++17 "-$pedOneShotsOptimization" @pedOneShotsArgs "$pedOneShotsBuild/after.cpp" @pedOneShotsCommon "$pedOneShotsBuild/AudioPedOneShots.o" "$pedOneShotsBuild/caller.o" -o "$pedOneShotsBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $pedOneShotsBefore=& "$pedOneShotsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $pedOneShotsAfter=& "$pedOneShotsBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $pedOneShotsBefore -ne $pedOneShotsAfter) {throw "PedOneShots comparison failed: $pedOneShotsBefore / $pedOneShotsAfter"}
   $pedOneShotsRuns += "$pedOneShotsCompiler-$pedOneShotsOptimization-$pedOneShotsShape : $pedOneShotsAfter"
   Write-Output $pedOneShotsRuns[-1]
  }
 }
}
$pedOneShotsRuns | ConvertTo-Json | Set-Content "$pedOneShotsBuild/runtime-runs.json"
$pedOneShotsSanBuild=Join-Path $pedOneShotsBuild 'sanitized'
New-Item -ItemType Directory -Path $pedOneShotsSanBuild -Force | Out-Null
$pedOneShotsSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$pedOneShotsRoot/src/audio","-I$pedOneShotsRoot/src/core",'-include',"$pedOneShotsBuild/default.h")
foreach($pedOneShotsSanUnit in @('AudioPedOneShots','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @pedOneShotsSanArgs -c "$pedOneShotsRoot/src/audio/$pedOneShotsSanUnit.c" $(if($pedOneShotsSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestPedOneShotsRealSubmit'}) -o "$pedOneShotsSanBuild/$pedOneShotsSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $pedOneShotsSanUnit"}
}
& clang -std=c11 @pedOneShotsSanArgs -c "$pedOneShotsBuild/caller.c" -o "$pedOneShotsSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @pedOneShotsSanArgs -c "$pedOneShotsBuild/fast_hash.c" -o "$pedOneShotsSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$pedOneShotsSanCommon=@("$pedOneShotsSanBuild/fast_hash.o","$pedOneShotsSanBuild/AudioMath.o","$pedOneShotsSanBuild/AudioGeometry.o","$pedOneShotsSanBuild/AudioRequests.o","$pedOneShotsSanBuild/AudioSoundQueue.o")
$pedOneShotsSanResults=@()
foreach($pedOneShotsSanVariant in @('before','after')) {
 $pedOneShotsSanExtra=if($pedOneShotsSanVariant -eq 'after') {@("$pedOneShotsSanBuild/AudioPedOneShots.o","$pedOneShotsSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @pedOneShotsSanArgs "$pedOneShotsBuild/$pedOneShotsSanVariant.cpp" @pedOneShotsSanCommon @pedOneShotsSanExtra -o "$pedOneShotsSanBuild/$pedOneShotsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $pedOneShotsSanResults += & "$pedOneShotsSanBuild/$pedOneShotsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $pedOneShotsSanVariant"}
}
if($pedOneShotsSanResults.Count -ne 2 -or $pedOneShotsSanResults[0] -ne $pedOneShotsSanResults[1]) {throw 'Sanitized comparison failed'}
$pedOneShotsSanResults | ConvertTo-Json | Set-Content "$pedOneShotsBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($pedOneShotsSanResults[1])"
