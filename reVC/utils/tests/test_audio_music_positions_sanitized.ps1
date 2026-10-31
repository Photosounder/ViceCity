$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicPositionsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicPositionsBuild=Join-Path $musicPositionsRoot 'build/audio-music-positions-c-tests'
$musicPositionsSources=$musicPositionsBuild
$musicPositionsBuild=Join-Path $musicPositionsBuild 'sanitized'
New-Item -ItemType Directory -Force $musicPositionsBuild | Out-Null
$musicPositionsRuns=@()
foreach($musicPositionsCompiler in @('clang')) {
 foreach($musicPositionsOptimization in @('O1')) {
  $musicPositionsCpp=if($musicPositionsCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicPositionsShape in @('default','ps2')) {
   $musicPositionsArgs=@('-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$musicPositionsRoot/src/audio","-I$musicPositionsRoot/src/core",'-include',"$musicPositionsSources/$musicPositionsShape.h")
   foreach($musicPositionsUnit in @('MusicPositions','MusicState')) {
    & $musicPositionsCompiler -std=c11 -pedantic-errors "-$musicPositionsOptimization" @musicPositionsArgs -c "$musicPositionsRoot/src/audio/$musicPositionsUnit.c" -o "$musicPositionsBuild/$musicPositionsUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicPositionsUnit"}
   }
   & $musicPositionsCompiler -std=c11 -pedantic-errors "-$musicPositionsOptimization" @musicPositionsArgs -c "$musicPositionsSources/caller.c" -o "$musicPositionsBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicPositionsCompiler -std=c11 -O2 -c "$musicPositionsSources/fast_hash.c" -o "$musicPositionsBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicPositionsCommon=@("$musicPositionsBuild/fast_hash.o","$musicPositionsBuild/MusicState.o")
   & $musicPositionsCpp -std=c++17 "-$musicPositionsOptimization" @musicPositionsArgs "$musicPositionsSources/before.cpp" @musicPositionsCommon -o "$musicPositionsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicPositionsCpp -std=c++17 "-$musicPositionsOptimization" @musicPositionsArgs "$musicPositionsSources/after.cpp" @musicPositionsCommon "$musicPositionsBuild/MusicPositions.o" "$musicPositionsBuild/caller.o" -o "$musicPositionsBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $musicPositionsBefore=& "$musicPositionsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $musicPositionsAfter=& "$musicPositionsBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $musicPositionsBefore -ne $musicPositionsAfter) {throw "MusicPositions comparison failed: $musicPositionsBefore / $musicPositionsAfter"}
   $musicPositionsRuns += "$musicPositionsCompiler-$musicPositionsOptimization-$musicPositionsShape : $musicPositionsAfter"
   Write-Output $musicPositionsRuns[-1]
  }
 }
}
$musicPositionsRuns | ConvertTo-Json | Set-Content "$musicPositionsBuild/runtime-runs.json"
