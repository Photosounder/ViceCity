$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$aircraftRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$aircraftBuild=Join-Path $aircraftRoot 'build/audio-aircraft-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_aircraft_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Aircraft source audit failed'}
$aircraftRuns=@()
foreach($aircraftCompiler in @('clang','gcc')) {
 foreach($aircraftOptimization in @('O0','O2')) {
  $aircraftCpp=if($aircraftCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($aircraftShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale')) {
   $aircraftArgs=@("-I$aircraftRoot/src/audio","-I$aircraftRoot/src/core",'-include',"$aircraftBuild/$aircraftShape.h")
   foreach($aircraftUnit in @('AudioAircraft','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
    & $aircraftCompiler -std=c11 -pedantic-errors "-$aircraftOptimization" @aircraftArgs -c "$aircraftRoot/src/audio/$aircraftUnit.c" -o "$aircraftBuild/$aircraftUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $aircraftUnit"}
   }
   & $aircraftCompiler -std=c11 -pedantic-errors "-$aircraftOptimization" @aircraftArgs -c "$aircraftBuild/caller.c" -o "$aircraftBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $aircraftCompiler -std=c11 -O2 -c "$aircraftBuild/fast_hash.c" -o "$aircraftBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $aircraftCommon=@("$aircraftBuild/fast_hash.o","$aircraftBuild/AudioMath.o","$aircraftBuild/AudioGeometry.o","$aircraftBuild/AudioRequests.o","$aircraftBuild/AudioSoundQueue.o")
   & $aircraftCpp -std=c++17 "-$aircraftOptimization" @aircraftArgs "$aircraftBuild/before.cpp" @aircraftCommon -o "$aircraftBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $aircraftCpp -std=c++17 "-$aircraftOptimization" @aircraftArgs "$aircraftBuild/after.cpp" @aircraftCommon "$aircraftBuild/AudioAircraft.o" "$aircraftBuild/caller.o" -o "$aircraftBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $aircraftBefore=& "$aircraftBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $aircraftAfter=& "$aircraftBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $aircraftBefore -ne $aircraftAfter) {throw "Aircraft comparison failed: $aircraftBefore / $aircraftAfter"}
   $aircraftRuns += "$aircraftCompiler-$aircraftOptimization-$aircraftShape : $aircraftAfter"
   Write-Output $aircraftRuns[-1]
  }
 }
}
$aircraftRuns | ConvertTo-Json | Set-Content "$aircraftBuild/runtime-runs.json"
$aircraftSanBuild=Join-Path $aircraftBuild 'sanitized'
New-Item -ItemType Directory -Path $aircraftSanBuild -Force | Out-Null
$aircraftSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$aircraftRoot/src/audio","-I$aircraftRoot/src/core",'-include',"$aircraftBuild/default.h")
foreach($aircraftSanUnit in @('AudioAircraft','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @aircraftSanArgs -c "$aircraftRoot/src/audio/$aircraftSanUnit.c" -o "$aircraftSanBuild/$aircraftSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $aircraftSanUnit"}
}
& clang -std=c11 @aircraftSanArgs -c "$aircraftBuild/caller.c" -o "$aircraftSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @aircraftSanArgs -c "$aircraftBuild/fast_hash.c" -o "$aircraftSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$aircraftSanCommon=@("$aircraftSanBuild/fast_hash.o","$aircraftSanBuild/AudioMath.o","$aircraftSanBuild/AudioGeometry.o","$aircraftSanBuild/AudioRequests.o","$aircraftSanBuild/AudioSoundQueue.o")
$aircraftSanResults=@()
foreach($aircraftSanVariant in @('before','after')) {
 $aircraftSanExtra=if($aircraftSanVariant -eq 'after') {@("$aircraftSanBuild/AudioAircraft.o","$aircraftSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @aircraftSanArgs "$aircraftBuild/$aircraftSanVariant.cpp" @aircraftSanCommon @aircraftSanExtra -o "$aircraftSanBuild/$aircraftSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $aircraftSanResults += & "$aircraftSanBuild/$aircraftSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $aircraftSanVariant"}
}
if($aircraftSanResults.Count -ne 2 -or $aircraftSanResults[0] -ne $aircraftSanResults[1]) {throw 'Sanitized comparison failed'}
$aircraftSanResults | ConvertTo-Json | Set-Content "$aircraftBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($aircraftSanResults[1])"
