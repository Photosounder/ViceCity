$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicModesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicModesBuild=Join-Path $musicModesRoot 'build/audio-music-modes-c-tests'
$musicModesSources=$musicModesBuild
$musicModesBuild=Join-Path $musicModesBuild 'sanitized'
New-Item -ItemType Directory -Force $musicModesBuild | Out-Null
$musicModesRuns=@()
foreach($musicModesCompiler in @('clang')) {
 foreach($musicModesOptimization in @('O1')) {
  $musicModesCpp=if($musicModesCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicModesShape in @('default','ps2','no-pause-radio')) {
   $musicModesArgs=@('-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$musicModesRoot/src/audio","-I$musicModesRoot/src/core",'-include',"$musicModesSources/$musicModesShape.h")
   foreach($musicModesUnit in @('MusicModes','MusicState','MusicPositions')) {
    & $musicModesCompiler -std=c11 -pedantic-errors "-$musicModesOptimization" @musicModesArgs -c "$musicModesRoot/src/audio/$musicModesUnit.c" -o "$musicModesBuild/$musicModesUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicModesUnit"}
   }
   & $musicModesCompiler -std=c11 -pedantic-errors "-$musicModesOptimization" @musicModesArgs -c "$musicModesSources/caller.c" -o "$musicModesBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicModesCompiler -std=c11 -O2 -c "$musicModesSources/fast_hash.c" -o "$musicModesBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicModesCommon=@("$musicModesBuild/fast_hash.o","$musicModesBuild/MusicState.o","$musicModesBuild/MusicPositions.o")
   & $musicModesCpp -std=c++17 "-$musicModesOptimization" @musicModesArgs "$musicModesSources/before.cpp" @musicModesCommon -o "$musicModesBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicModesCpp -std=c++17 "-$musicModesOptimization" @musicModesArgs "$musicModesSources/after.cpp" @musicModesCommon "$musicModesBuild/MusicModes.o" "$musicModesBuild/caller.o" -o "$musicModesBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $musicModesBefore=& "$musicModesBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $musicModesAfter=& "$musicModesBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $musicModesBefore -ne $musicModesAfter) {throw "MusicModes comparison failed: $musicModesBefore / $musicModesAfter"}
   $musicModesRuns += "$musicModesCompiler-$musicModesOptimization-$musicModesShape : $musicModesAfter"
   Write-Output $musicModesRuns[-1]
  }
 }
}
$musicModesRuns | ConvertTo-Json | Set-Content "$musicModesBuild/runtime-runs.json"
