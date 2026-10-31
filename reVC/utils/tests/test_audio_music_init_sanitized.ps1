$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicInitRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicInitBuild=Join-Path $musicInitRoot 'build/audio-music-init-c-tests'
$musicInitSources=$musicInitBuild
$musicInitBuild=Join-Path $musicInitBuild 'sanitized'
New-Item -ItemType Directory -Force $musicInitBuild | Out-Null
$musicInitRuns=@()
foreach($musicInitCompiler in @('clang')) {
 foreach($musicInitOptimization in @('O1')) {
  $musicInitCpp=if($musicInitCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicInitShape in @('default','ps2')) {
   $musicInitArgs=@('-DTEST_MUSIC_SAFE','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$musicInitRoot/src/audio","-I$musicInitRoot/src/core",'-include',"$musicInitSources/$musicInitShape.h",'-include',"$musicInitSources/clock.h")
   foreach($musicInitUnit in @('MusicInit','MusicState')) {
    & $musicInitCompiler -std=c11 -pedantic-errors "-$musicInitOptimization" @musicInitArgs -c "$musicInitRoot/src/audio/$musicInitUnit.c" -o "$musicInitBuild/$musicInitUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicInitUnit"}
   }
   & $musicInitCompiler -std=c11 -pedantic-errors "-$musicInitOptimization" @musicInitArgs -c "$musicInitSources/caller.c" -o "$musicInitBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicInitCompiler -std=c11 -O2 -c "$musicInitSources/fast_hash.c" -o "$musicInitBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicInitCommon=@("$musicInitBuild/fast_hash.o","$musicInitBuild/MusicState.o")
   & $musicInitCpp -std=c++17 "-$musicInitOptimization" @musicInitArgs "$musicInitSources/before.cpp" @musicInitCommon -o "$musicInitBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicInitCpp -std=c++17 "-$musicInitOptimization" @musicInitArgs "$musicInitSources/after.cpp" @musicInitCommon "$musicInitBuild/MusicInit.o" "$musicInitBuild/caller.o" -o "$musicInitBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $musicInitBefore=& "$musicInitBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $musicInitAfter=& "$musicInitBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $musicInitBefore -ne $musicInitAfter) {throw "MusicInit comparison failed: $musicInitBefore / $musicInitAfter"}
   $musicInitRuns += "$musicInitCompiler-$musicInitOptimization-$musicInitShape : $musicInitAfter"
   Write-Output $musicInitRuns[-1]
  }
 }
}
$musicInitRuns | ConvertTo-Json | Set-Content "$musicInitBuild/runtime-runs.json"
