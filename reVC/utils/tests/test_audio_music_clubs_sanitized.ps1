$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicClubsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicClubsBuild=Join-Path $musicClubsRoot 'build/audio-music-clubs-c-tests'
$musicClubsSources=$musicClubsBuild
$musicClubsBuild=Join-Path $musicClubsBuild 'sanitized'
New-Item -ItemType Directory -Force $musicClubsBuild | Out-Null
$musicClubsRuns=@()
foreach($musicClubsCompiler in @('clang')) {
 foreach($musicClubsOptimization in @('O1')) {
  $musicClubsCpp=if($musicClubsCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicClubsShape in @('default','ps2')) {
   $musicClubsArgs=@('-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$musicClubsRoot/src/audio","-I$musicClubsRoot/src/core",'-include',"$musicClubsSources/$musicClubsShape.h")
   foreach($musicClubsUnit in @('MusicClubs','MusicState')) {
    & $musicClubsCompiler -std=c11 -pedantic-errors "-$musicClubsOptimization" @musicClubsArgs -c "$musicClubsRoot/src/audio/$musicClubsUnit.c" -o "$musicClubsBuild/$musicClubsUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicClubsUnit"}
   }
   & $musicClubsCompiler -std=c11 -pedantic-errors "-$musicClubsOptimization" @musicClubsArgs -c "$musicClubsSources/caller.c" -o "$musicClubsBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicClubsCompiler -std=c11 -O2 -c "$musicClubsSources/fast_hash.c" -o "$musicClubsBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicClubsCommon=@("$musicClubsBuild/fast_hash.o","$musicClubsBuild/MusicState.o")
   & $musicClubsCpp -std=c++17 "-$musicClubsOptimization" @musicClubsArgs "$musicClubsSources/before.cpp" @musicClubsCommon -o "$musicClubsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicClubsCpp -std=c++17 "-$musicClubsOptimization" @musicClubsArgs "$musicClubsSources/after.cpp" @musicClubsCommon "$musicClubsBuild/MusicClubs.o" "$musicClubsBuild/caller.o" -o "$musicClubsBuild/after.exe"
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
