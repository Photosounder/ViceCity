$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicPlaybackRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicPlaybackBuild=Join-Path $musicPlaybackRoot 'build/audio-music-playback-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_music_playback_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'MusicPlayback source audit failed'}
$musicPlaybackRuns=@()
foreach($musicPlaybackCompiler in @('clang','gcc')) {
 foreach($musicPlaybackOptimization in @('O0','O2')) {
  $musicPlaybackCpp=if($musicPlaybackCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicPlaybackShape in @('default','vanilla','no-external','ps2','ps2-no-reverb')) {
   $musicPlaybackArgs=@("-I$musicPlaybackRoot/src/audio","-I$musicPlaybackRoot/src/core",'-include',"$musicPlaybackBuild/$musicPlaybackShape.h")
   foreach($musicPlaybackUnit in @('MusicPlayback','MusicState','AudioPoliceState')) {
    & $musicPlaybackCompiler -std=c11 -pedantic-errors "-$musicPlaybackOptimization" @musicPlaybackArgs -c "$musicPlaybackRoot/src/audio/$musicPlaybackUnit.c" -o "$musicPlaybackBuild/$musicPlaybackUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicPlaybackUnit"}
   }
   & $musicPlaybackCompiler -std=c11 -pedantic-errors "-$musicPlaybackOptimization" @musicPlaybackArgs -c "$musicPlaybackBuild/caller.c" -o "$musicPlaybackBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicPlaybackCompiler -std=c11 -O2 -c "$musicPlaybackBuild/fast_hash.c" -o "$musicPlaybackBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicPlaybackCommon=@("$musicPlaybackBuild/fast_hash.o","$musicPlaybackBuild/MusicState.o","$musicPlaybackBuild/AudioPoliceState.o")
   & $musicPlaybackCpp -std=c++17 "-$musicPlaybackOptimization" @musicPlaybackArgs "$musicPlaybackBuild/before.cpp" @musicPlaybackCommon -o "$musicPlaybackBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicPlaybackCpp -std=c++17 "-$musicPlaybackOptimization" @musicPlaybackArgs "$musicPlaybackBuild/after.cpp" @musicPlaybackCommon "$musicPlaybackBuild/MusicPlayback.o" "$musicPlaybackBuild/caller.o" -o "$musicPlaybackBuild/after.exe"
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
