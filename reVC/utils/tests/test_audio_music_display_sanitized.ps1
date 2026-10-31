$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicDisplayRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicDisplayBuild=Join-Path $musicDisplayRoot 'build/audio-music-display-c-tests'

$musicDisplaySources=$musicDisplayBuild
$musicDisplayBuild=Join-Path $musicDisplayBuild 'sanitized'
New-Item -ItemType Directory -Force $musicDisplayBuild | Out-Null
$musicDisplayRuns=@()
foreach($musicDisplayCompiler in @('clang')) {
 foreach($musicDisplayOptimization in @('O1')) {
  $musicDisplayCpp=if($musicDisplayCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicDisplayShape in @('default','vanilla','no-aspect')) {
   $musicDisplayArgs=@('-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$musicDisplayRoot/src/audio","-I$musicDisplayRoot/src/core",'-include',"$musicDisplaySources/$musicDisplayShape.h")
   foreach($musicDisplayUnit in @('MusicDisplay')) {
    & $musicDisplayCompiler -std=c11 -pedantic-errors "-$musicDisplayOptimization" @musicDisplayArgs -c "$musicDisplayRoot/src/audio/$musicDisplayUnit.c" -o "$musicDisplayBuild/$musicDisplayUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicDisplayUnit"}
   }
   & $musicDisplayCompiler -std=c11 -pedantic-errors "-$musicDisplayOptimization" @musicDisplayArgs -c "$musicDisplaySources/caller.c" -o "$musicDisplayBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicDisplayCompiler -std=c11 -O2 -c "$musicDisplaySources/fast_hash.c" -o "$musicDisplayBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicDisplayCommon=@("$musicDisplayBuild/fast_hash.o")
   & $musicDisplayCpp -std=c++17 "-$musicDisplayOptimization" @musicDisplayArgs "$musicDisplaySources/before.cpp" @musicDisplayCommon -o "$musicDisplayBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicDisplayCpp -std=c++17 "-$musicDisplayOptimization" @musicDisplayArgs "$musicDisplaySources/after.cpp" @musicDisplayCommon "$musicDisplayBuild/MusicDisplay.o" "$musicDisplayBuild/caller.o" -o "$musicDisplayBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $musicDisplayBefore=& "$musicDisplayBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $musicDisplayAfter=& "$musicDisplayBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $musicDisplayBefore -ne $musicDisplayAfter) {throw "MusicDisplay comparison failed: $musicDisplayBefore / $musicDisplayAfter"}
   $musicDisplayRuns += "$musicDisplayCompiler-$musicDisplayOptimization-$musicDisplayShape : $musicDisplayAfter"
   Write-Output $musicDisplayRuns[-1]
  }
 }
}
$musicDisplayRuns | ConvertTo-Json | Set-Content "$musicDisplayBuild/runtime-runs.json"
