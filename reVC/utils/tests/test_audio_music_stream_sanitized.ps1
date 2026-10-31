$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicStreamRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicStreamBuild=Join-Path $musicStreamRoot 'build/audio-music-stream-c-tests'
$musicStreamSources=$musicStreamBuild
$musicStreamBuild=Join-Path $musicStreamBuild 'sanitized'
New-Item -ItemType Directory -Force $musicStreamBuild | Out-Null
$musicStreamRuns=@()
foreach($musicStreamCompiler in @('clang')) {
 foreach($musicStreamOptimization in @('O1')) {
  $musicStreamCpp=if($musicStreamCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicStreamShape in @('default','ps2')) {
   $musicStreamArgs=@('-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$musicStreamRoot/src/audio","-I$musicStreamRoot/src/core",'-include',"$musicStreamSources/$musicStreamShape.h")
   foreach($musicStreamUnit in @('MusicStream','MusicState','MusicPositions','MusicAmbience','AudioGeometry','AudioMath','AudioMissionState','AudioMissionPosition')) {
    & $musicStreamCompiler -std=c11 -pedantic-errors "-$musicStreamOptimization" @musicStreamArgs -c "$musicStreamRoot/src/audio/$musicStreamUnit.c" -o "$musicStreamBuild/$musicStreamUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicStreamUnit"}
   }
   & $musicStreamCompiler -std=c11 -pedantic-errors "-$musicStreamOptimization" @musicStreamArgs -c "$musicStreamSources/caller.c" -o "$musicStreamBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicStreamCompiler -std=c11 -O2 -c "$musicStreamSources/fast_hash.c" -o "$musicStreamBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicStreamCommon=@("$musicStreamBuild/fast_hash.o","$musicStreamBuild/MusicState.o","$musicStreamBuild/MusicPositions.o","$musicStreamBuild/MusicAmbience.o","$musicStreamBuild/AudioGeometry.o","$musicStreamBuild/AudioMath.o","$musicStreamBuild/AudioMissionState.o","$musicStreamBuild/AudioMissionPosition.o")
   & $musicStreamCpp -std=c++17 "-$musicStreamOptimization" @musicStreamArgs "$musicStreamSources/before.cpp" @musicStreamCommon -o "$musicStreamBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicStreamCpp -std=c++17 "-$musicStreamOptimization" @musicStreamArgs "$musicStreamSources/after.cpp" @musicStreamCommon "$musicStreamBuild/MusicStream.o" "$musicStreamBuild/caller.o" -o "$musicStreamBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $musicStreamBefore=& "$musicStreamBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $musicStreamAfter=& "$musicStreamBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $musicStreamBefore -ne $musicStreamAfter) {throw "MusicStream comparison failed: $musicStreamBefore / $musicStreamAfter"}
   $musicStreamRuns += "$musicStreamCompiler-$musicStreamOptimization-$musicStreamShape : $musicStreamAfter"
   Write-Output $musicStreamRuns[-1]
  }
 }
}
$musicStreamRuns | ConvertTo-Json | Set-Content "$musicStreamBuild/runtime-runs.json"
