$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicStateRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicStateBuild=Join-Path $musicStateRoot 'build/audio-music-state-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_music_state_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'MusicState source audit failed'}
$musicStateRuns=@()
foreach($musicStateCompiler in @('clang','gcc')) {
 foreach($musicStateOptimization in @('O0','O2')) {
  $musicStateCpp=if($musicStateCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicStateShape in @('default','vanilla','no-external','ps2')) {
   $musicStateArgs=@("-I$musicStateRoot/src/audio","-I$musicStateRoot/src/core",'-include',"$musicStateBuild/$musicStateShape.h")
   foreach($musicStateUnit in @('MusicState')) {
    & $musicStateCompiler -std=c11 -pedantic-errors "-$musicStateOptimization" @musicStateArgs -c "$musicStateRoot/src/audio/$musicStateUnit.c" -o "$musicStateBuild/$musicStateUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicStateUnit"}
   }
   & $musicStateCompiler -std=c11 -pedantic-errors "-$musicStateOptimization" @musicStateArgs -c "$musicStateBuild/caller.c" -o "$musicStateBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicStateCompiler -std=c11 -O2 -c "$musicStateBuild/fast_hash.c" -o "$musicStateBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicStateCommon=@("$musicStateBuild/fast_hash.o")
   & $musicStateCpp -std=c++17 "-$musicStateOptimization" @musicStateArgs "$musicStateBuild/before.cpp" @musicStateCommon -o "$musicStateBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicStateCpp -std=c++17 "-$musicStateOptimization" @musicStateArgs "$musicStateBuild/after.cpp" @musicStateCommon "$musicStateBuild/MusicState.o" "$musicStateBuild/caller.o" -o "$musicStateBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $musicStateBefore=& "$musicStateBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $musicStateAfter=& "$musicStateBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $musicStateBefore -ne $musicStateAfter) {throw "MusicState comparison failed: $musicStateBefore / $musicStateAfter"}
   $musicStateRuns += "$musicStateCompiler-$musicStateOptimization-$musicStateShape : $musicStateAfter"
   Write-Output $musicStateRuns[-1]
  }
 }
}
$musicStateRuns | ConvertTo-Json | Set-Content "$musicStateBuild/runtime-runs.json"
