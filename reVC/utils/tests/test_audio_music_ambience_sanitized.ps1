$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicAmbienceRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicAmbienceBuild=Join-Path $musicAmbienceRoot 'build/audio-music-ambience-c-tests'
$musicAmbienceSources=$musicAmbienceBuild
$musicAmbienceBuild=Join-Path $musicAmbienceBuild 'sanitized'
New-Item -ItemType Directory -Force $musicAmbienceBuild | Out-Null
$musicAmbienceRuns=@()
foreach($musicAmbienceCompiler in @('clang')) {
 foreach($musicAmbienceOptimization in @('O1')) {
  $musicAmbienceCpp=if($musicAmbienceCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicAmbienceShape in @('default','ps2')) {
   $musicAmbienceArgs=@('-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$musicAmbienceRoot/src/audio","-I$musicAmbienceRoot/src/core",'-include',"$musicAmbienceSources/$musicAmbienceShape.h")
   foreach($musicAmbienceUnit in @('MusicAmbience')) {
    & $musicAmbienceCompiler -std=c11 -pedantic-errors "-$musicAmbienceOptimization" @musicAmbienceArgs -c "$musicAmbienceRoot/src/audio/$musicAmbienceUnit.c" -o "$musicAmbienceBuild/$musicAmbienceUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicAmbienceUnit"}
   }
   & $musicAmbienceCompiler -std=c11 -pedantic-errors "-$musicAmbienceOptimization" @musicAmbienceArgs -c "$musicAmbienceSources/caller.c" -o "$musicAmbienceBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicAmbienceCompiler -std=c11 -O2 -c "$musicAmbienceSources/fast_hash.c" -o "$musicAmbienceBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicAmbienceCommon=@("$musicAmbienceBuild/fast_hash.o")
   & $musicAmbienceCpp -std=c++17 "-$musicAmbienceOptimization" @musicAmbienceArgs "$musicAmbienceSources/before.cpp" @musicAmbienceCommon -o "$musicAmbienceBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicAmbienceCpp -std=c++17 "-$musicAmbienceOptimization" @musicAmbienceArgs "$musicAmbienceSources/after.cpp" @musicAmbienceCommon "$musicAmbienceBuild/MusicAmbience.o" "$musicAmbienceBuild/caller.o" -o "$musicAmbienceBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $musicAmbienceBefore=& "$musicAmbienceBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $musicAmbienceAfter=& "$musicAmbienceBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $musicAmbienceBefore -ne $musicAmbienceAfter) {throw "MusicAmbience comparison failed: $musicAmbienceBefore / $musicAmbienceAfter"}
   $musicAmbienceRuns += "$musicAmbienceCompiler-$musicAmbienceOptimization-$musicAmbienceShape : $musicAmbienceAfter"
   Write-Output $musicAmbienceRuns[-1]
  }
 }
}
$musicAmbienceRuns | ConvertTo-Json | Set-Content "$musicAmbienceBuild/runtime-runs.json"
