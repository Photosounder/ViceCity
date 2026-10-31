$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicClubsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicClubsBuild=Join-Path $musicClubsRoot 'build/audio-music-clubs-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_music_clubs_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'MusicClubs source audit failed'}
$musicClubsRuns=@()
foreach($musicClubsCompiler in @('clang','gcc')) {
 foreach($musicClubsOptimization in @('O0','O2')) {
  $musicClubsCpp=if($musicClubsCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicClubsShape in @('default','vanilla','no-external','ps2')) {
   $musicClubsArgs=@("-I$musicClubsRoot/src/audio","-I$musicClubsRoot/src/core",'-include',"$musicClubsBuild/$musicClubsShape.h")
   foreach($musicClubsUnit in @('MusicClubs','MusicState')) {
    & $musicClubsCompiler -std=c11 -pedantic-errors "-$musicClubsOptimization" @musicClubsArgs -c "$musicClubsRoot/src/audio/$musicClubsUnit.c" -o "$musicClubsBuild/$musicClubsUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicClubsUnit"}
   }
   & $musicClubsCompiler -std=c11 -pedantic-errors "-$musicClubsOptimization" @musicClubsArgs -c "$musicClubsBuild/caller.c" -o "$musicClubsBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicClubsCompiler -std=c11 -O2 -c "$musicClubsBuild/fast_hash.c" -o "$musicClubsBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicClubsCommon=@("$musicClubsBuild/fast_hash.o","$musicClubsBuild/MusicState.o")
   & $musicClubsCpp -std=c++17 "-$musicClubsOptimization" @musicClubsArgs "$musicClubsBuild/before.cpp" @musicClubsCommon -o "$musicClubsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicClubsCpp -std=c++17 "-$musicClubsOptimization" @musicClubsArgs "$musicClubsBuild/after.cpp" @musicClubsCommon "$musicClubsBuild/MusicClubs.o" "$musicClubsBuild/caller.o" -o "$musicClubsBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $musicClubsBefore=& "$musicClubsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $musicClubsAfter=& "$musicClubsBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $musicClubsBefore -ne $musicClubsAfter) {throw "MusicClubs comparison failed: $musicClubsBefore / $musicClubsAfter"}
   $musicClubsRuns += "$musicClubsCompiler-$musicClubsOptimization-$musicClubsShape : $musicClubsAfter"
   Write-Output $musicClubsRuns[-1]
  }
 }
}
$musicClubsRuns | ConvertTo-Json | Set-Content "$musicClubsBuild/runtime-runs.json"
