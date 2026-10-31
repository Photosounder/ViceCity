$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicStateRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicStateBuild=Join-Path $musicStateRoot 'build/audio-music-state-c-tests'
$musicStateSources=$musicStateBuild
$musicStateBuild=Join-Path $musicStateBuild 'sanitized'
New-Item -ItemType Directory -Force $musicStateBuild | Out-Null
$musicStateRuns=@()
foreach($musicStateCompiler in @('clang')) {
 foreach($musicStateOptimization in @('O1')) {
  $musicStateCpp=if($musicStateCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicStateShape in @('default','ps2')) {
   $musicStateArgs=@('-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$musicStateRoot/src/audio","-I$musicStateRoot/src/core",'-include',"$musicStateSources/$musicStateShape.h")
   foreach($musicStateUnit in @('MusicState')) {
    & $musicStateCompiler -std=c11 -pedantic-errors "-$musicStateOptimization" @musicStateArgs -c "$musicStateRoot/src/audio/$musicStateUnit.c" -o "$musicStateBuild/$musicStateUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicStateUnit"}
   }
   & $musicStateCompiler -std=c11 -pedantic-errors "-$musicStateOptimization" @musicStateArgs -c "$musicStateSources/caller.c" -o "$musicStateBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicStateCompiler -std=c11 -O2 -c "$musicStateSources/fast_hash.c" -o "$musicStateBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicStateCommon=@("$musicStateBuild/fast_hash.o")
   & $musicStateCpp -std=c++17 "-$musicStateOptimization" @musicStateArgs "$musicStateSources/before.cpp" @musicStateCommon -o "$musicStateBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicStateCpp -std=c++17 "-$musicStateOptimization" @musicStateArgs "$musicStateSources/after.cpp" @musicStateCommon "$musicStateBuild/MusicState.o" "$musicStateBuild/caller.o" -o "$musicStateBuild/after.exe"
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
