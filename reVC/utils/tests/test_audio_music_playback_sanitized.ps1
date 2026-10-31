$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicPlaybackRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicPlaybackBuild=Join-Path $musicPlaybackRoot 'build/audio-music-playback-c-tests'
$musicPlaybackSources=$musicPlaybackBuild
$musicPlaybackBuild=Join-Path $musicPlaybackBuild 'sanitized'
New-Item -ItemType Directory -Force $musicPlaybackBuild | Out-Null
$musicPlaybackRuns=@()
foreach($musicPlaybackCompiler in @('clang')) {
 foreach($musicPlaybackOptimization in @('O1')) {
  $musicPlaybackCpp=if($musicPlaybackCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicPlaybackShape in @('default','ps2-no-reverb')) {
   $musicPlaybackArgs=@('-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$musicPlaybackRoot/src/audio","-I$musicPlaybackRoot/src/core",'-include',"$musicPlaybackSources/$musicPlaybackShape.h")
   foreach($musicPlaybackUnit in @('MusicPlayback','MusicState','AudioPoliceState')) {
    & $musicPlaybackCompiler -std=c11 -pedantic-errors "-$musicPlaybackOptimization" @musicPlaybackArgs -c "$musicPlaybackRoot/src/audio/$musicPlaybackUnit.c" -o "$musicPlaybackBuild/$musicPlaybackUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicPlaybackUnit"}
   }
   & $musicPlaybackCompiler -std=c11 -pedantic-errors "-$musicPlaybackOptimization" @musicPlaybackArgs -c "$musicPlaybackSources/caller.c" -o "$musicPlaybackBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicPlaybackCompiler -std=c11 -O2 -c "$musicPlaybackSources/fast_hash.c" -o "$musicPlaybackBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicPlaybackCommon=@("$musicPlaybackBuild/fast_hash.o","$musicPlaybackBuild/MusicState.o","$musicPlaybackBuild/AudioPoliceState.o")
   & $musicPlaybackCpp -std=c++17 "-$musicPlaybackOptimization" @musicPlaybackArgs "$musicPlaybackSources/before.cpp" @musicPlaybackCommon -o "$musicPlaybackBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicPlaybackCpp -std=c++17 "-$musicPlaybackOptimization" @musicPlaybackArgs "$musicPlaybackSources/after.cpp" @musicPlaybackCommon "$musicPlaybackBuild/MusicPlayback.o" "$musicPlaybackBuild/caller.o" -o "$musicPlaybackBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $musicPlaybackBefore=& "$musicPlaybackBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $musicPlaybackAfter=& "$musicPlaybackBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $musicPlaybackBefore -ne $musicPlaybackAfter) {throw "MusicPlayback comparison failed: $musicPlaybackBefore / $musicPlaybackAfter"}
   $musicPlaybackRuns += "$musicPlaybackCompiler-$musicPlaybackOptimization-$musicPlaybackShape : $musicPlaybackAfter"
   Write-Output $musicPlaybackRuns[-1]
  }
 }
}
$musicPlaybackRuns | ConvertTo-Json | Set-Content "$musicPlaybackBuild/runtime-runs.json"
