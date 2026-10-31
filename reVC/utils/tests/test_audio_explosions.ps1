$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$explosionsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$explosionsBuild=Join-Path $explosionsRoot 'build/audio-explosions-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_explosions_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Explosion source audit failed'}
$explosionsRuns=@()
foreach($explosionsCompiler in @('clang','gcc')) {
 foreach($explosionsOptimization in @('O0','O2')) {
  $explosionsCpp=if($explosionsCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($explosionsShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale','no-reflections')) {
   $explosionsArgs=@("-I$explosionsRoot/src/audio","-I$explosionsRoot/src/core",'-include',"$explosionsBuild/$explosionsShape.h")
   foreach($explosionsUnit in @('AudioExplosions','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
    & $explosionsCompiler -std=c11 -pedantic-errors "-$explosionsOptimization" @explosionsArgs -c "$explosionsRoot/src/audio/$explosionsUnit.c" $(if($explosionsUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestExplosionsRealSubmit'}) -o "$explosionsBuild/$explosionsUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $explosionsUnit"}
   }
   & $explosionsCompiler -std=c11 -pedantic-errors "-$explosionsOptimization" @explosionsArgs -c "$explosionsBuild/caller.c" -o "$explosionsBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $explosionsCompiler -std=c11 -O2 -c "$explosionsBuild/fast_hash.c" -o "$explosionsBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $explosionsCommon=@("$explosionsBuild/fast_hash.o","$explosionsBuild/AudioMath.o","$explosionsBuild/AudioGeometry.o","$explosionsBuild/AudioRequests.o","$explosionsBuild/AudioSoundQueue.o")
   & $explosionsCpp -std=c++17 "-$explosionsOptimization" @explosionsArgs "$explosionsBuild/before.cpp" @explosionsCommon -o "$explosionsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $explosionsCpp -std=c++17 "-$explosionsOptimization" @explosionsArgs "$explosionsBuild/after.cpp" @explosionsCommon "$explosionsBuild/AudioExplosions.o" "$explosionsBuild/caller.o" -o "$explosionsBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $explosionsBefore=& "$explosionsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $explosionsAfter=& "$explosionsBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $explosionsBefore -ne $explosionsAfter) {throw "Explosion comparison failed: $explosionsBefore / $explosionsAfter"}
   $explosionsRuns += "$explosionsCompiler-$explosionsOptimization-$explosionsShape : $explosionsAfter"
   Write-Output $explosionsRuns[-1]
  }
 }
}
$explosionsRuns | ConvertTo-Json | Set-Content "$explosionsBuild/runtime-runs.json"
$explosionsSanBuild=Join-Path $explosionsBuild 'sanitized'
New-Item -ItemType Directory -Path $explosionsSanBuild -Force | Out-Null
$explosionsSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$explosionsRoot/src/audio","-I$explosionsRoot/src/core",'-include',"$explosionsBuild/default.h")
foreach($explosionsSanUnit in @('AudioExplosions','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @explosionsSanArgs -c "$explosionsRoot/src/audio/$explosionsSanUnit.c" $(if($explosionsSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestExplosionsRealSubmit'}) -o "$explosionsSanBuild/$explosionsSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $explosionsSanUnit"}
}
& clang -std=c11 @explosionsSanArgs -c "$explosionsBuild/caller.c" -o "$explosionsSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @explosionsSanArgs -c "$explosionsBuild/fast_hash.c" -o "$explosionsSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$explosionsSanCommon=@("$explosionsSanBuild/fast_hash.o","$explosionsSanBuild/AudioMath.o","$explosionsSanBuild/AudioGeometry.o","$explosionsSanBuild/AudioRequests.o","$explosionsSanBuild/AudioSoundQueue.o")
$explosionsSanResults=@()
foreach($explosionsSanVariant in @('before','after')) {
 $explosionsSanExtra=if($explosionsSanVariant -eq 'after') {@("$explosionsSanBuild/AudioExplosions.o","$explosionsSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @explosionsSanArgs "$explosionsBuild/$explosionsSanVariant.cpp" @explosionsSanCommon @explosionsSanExtra -o "$explosionsSanBuild/$explosionsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $explosionsSanResults += & "$explosionsSanBuild/$explosionsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $explosionsSanVariant"}
}
if($explosionsSanResults.Count -ne 2 -or $explosionsSanResults[0] -ne $explosionsSanResults[1]) {throw 'Sanitized comparison failed'}
$explosionsSanResults | ConvertTo-Json | Set-Content "$explosionsBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($explosionsSanResults[1])"
