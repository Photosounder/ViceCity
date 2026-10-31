$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicServiceRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicServiceBuild=Join-Path $musicServiceRoot 'build/audio-music-service-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_music_service_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'MusicService source audit failed'}
$musicServiceRuns=@()
foreach($musicServiceCompiler in @('clang','gcc')) {
 foreach($musicServiceOptimization in @('O0','O2')) {
  $musicServiceCpp=if($musicServiceCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicServiceShape in @('default','vanilla','no-external','ps2','no-previous-station')) {
   $musicServiceArgs=@("-I$musicServiceRoot/src/audio","-I$musicServiceRoot/src/core",'-include',"$musicServiceBuild/$musicServiceShape.h")
   foreach($musicServiceUnit in @('MusicService','MusicState','MusicPositions','MusicAmbience','MusicModes','MusicRadio','MusicStream','AudioGeometry','AudioMath','AudioMissionState','AudioMissionPosition')) {
    & $musicServiceCompiler -std=c11 -pedantic-errors "-$musicServiceOptimization" @musicServiceArgs -c "$musicServiceRoot/src/audio/$musicServiceUnit.c" -o "$musicServiceBuild/$musicServiceUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicServiceUnit"}
   }
   & $musicServiceCompiler -std=c11 -pedantic-errors "-$musicServiceOptimization" @musicServiceArgs -c "$musicServiceBuild/caller.c" -o "$musicServiceBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicServiceCompiler -std=c11 -O2 -c "$musicServiceBuild/fast_hash.c" -o "$musicServiceBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicServiceCommon=@("$musicServiceBuild/fast_hash.o","$musicServiceBuild/MusicState.o","$musicServiceBuild/MusicPositions.o","$musicServiceBuild/MusicAmbience.o","$musicServiceBuild/MusicModes.o","$musicServiceBuild/MusicRadio.o","$musicServiceBuild/MusicStream.o","$musicServiceBuild/AudioGeometry.o","$musicServiceBuild/AudioMath.o","$musicServiceBuild/AudioMissionState.o","$musicServiceBuild/AudioMissionPosition.o")
   & $musicServiceCpp -std=c++17 "-$musicServiceOptimization" @musicServiceArgs "$musicServiceBuild/before.cpp" @musicServiceCommon -o "$musicServiceBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicServiceCpp -std=c++17 "-$musicServiceOptimization" @musicServiceArgs "$musicServiceBuild/after.cpp" @musicServiceCommon "$musicServiceBuild/MusicService.o" "$musicServiceBuild/caller.o" -o "$musicServiceBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $musicServiceBefore=& "$musicServiceBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $musicServiceAfter=& "$musicServiceBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $musicServiceBefore -ne $musicServiceAfter) {throw "MusicService comparison failed: $musicServiceBefore / $musicServiceAfter"}
   $musicServiceRuns += "$musicServiceCompiler-$musicServiceOptimization-$musicServiceShape : $musicServiceAfter"
   Write-Output $musicServiceRuns[-1]
  }
 }
}
$musicServiceRuns | ConvertTo-Json | Set-Content "$musicServiceBuild/runtime-runs.json"
