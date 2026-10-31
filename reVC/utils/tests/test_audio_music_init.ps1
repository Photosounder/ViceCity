$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicInitRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicInitBuild=Join-Path $musicInitRoot 'build/audio-music-init-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_music_init_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'MusicInit source audit failed'}
$musicInitRuns=@()
foreach($musicInitCompiler in @('clang','gcc')) {
 foreach($musicInitOptimization in @('O0','O2')) {
  $musicInitCpp=if($musicInitCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicInitShape in @('default','vanilla','no-external','ps2')) {
   $musicInitArgs=@("-I$musicInitRoot/src/audio","-I$musicInitRoot/src/core",'-include',"$musicInitBuild/$musicInitShape.h",'-include',"$musicInitBuild/clock.h")
   foreach($musicInitUnit in @('MusicInit','MusicState')) {
    & $musicInitCompiler -std=c11 -pedantic-errors "-$musicInitOptimization" @musicInitArgs -c "$musicInitRoot/src/audio/$musicInitUnit.c" -o "$musicInitBuild/$musicInitUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicInitUnit"}
   }
   & $musicInitCompiler -std=c11 -pedantic-errors "-$musicInitOptimization" @musicInitArgs -c "$musicInitBuild/caller.c" -o "$musicInitBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicInitCompiler -std=c11 -O2 -c "$musicInitBuild/fast_hash.c" -o "$musicInitBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicInitCommon=@("$musicInitBuild/fast_hash.o","$musicInitBuild/MusicState.o")
   & $musicInitCpp -std=c++17 "-$musicInitOptimization" @musicInitArgs "$musicInitBuild/before.cpp" @musicInitCommon -o "$musicInitBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicInitCpp -std=c++17 "-$musicInitOptimization" @musicInitArgs "$musicInitBuild/after.cpp" @musicInitCommon "$musicInitBuild/MusicInit.o" "$musicInitBuild/caller.o" -o "$musicInitBuild/after.exe"
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
