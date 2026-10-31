$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicRadioRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicRadioBuild=Join-Path $musicRadioRoot 'build/audio-music-radio-c-tests'
$musicRadioSources=$musicRadioBuild
$musicRadioBuild=Join-Path $musicRadioBuild 'sanitized'
New-Item -ItemType Directory -Force $musicRadioBuild | Out-Null
$musicRadioRuns=@()
foreach($musicRadioCompiler in @('clang')) {
 foreach($musicRadioOptimization in @('O1')) {
  $musicRadioCpp=if($musicRadioCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicRadioShape in @('default','ps2','no-previous-station')) {
   $musicRadioArgs=@('-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$musicRadioRoot/src/audio","-I$musicRadioRoot/src/core",'-include',"$musicRadioSources/$musicRadioShape.h")
   foreach($musicRadioUnit in @('MusicRadio','MusicState')) {
    & $musicRadioCompiler -std=c11 -pedantic-errors "-$musicRadioOptimization" @musicRadioArgs -c "$musicRadioRoot/src/audio/$musicRadioUnit.c" -o "$musicRadioBuild/$musicRadioUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicRadioUnit"}
   }
   & $musicRadioCompiler -std=c11 -pedantic-errors "-$musicRadioOptimization" @musicRadioArgs -c "$musicRadioSources/caller.c" -o "$musicRadioBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicRadioCompiler -std=c11 -O2 -c "$musicRadioSources/fast_hash.c" -o "$musicRadioBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicRadioCommon=@("$musicRadioBuild/fast_hash.o","$musicRadioBuild/MusicState.o")
   & $musicRadioCpp -std=c++17 "-$musicRadioOptimization" @musicRadioArgs "$musicRadioSources/before.cpp" @musicRadioCommon -o "$musicRadioBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicRadioCpp -std=c++17 "-$musicRadioOptimization" @musicRadioArgs "$musicRadioSources/after.cpp" @musicRadioCommon "$musicRadioBuild/MusicRadio.o" "$musicRadioBuild/caller.o" -o "$musicRadioBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $musicRadioBefore=& "$musicRadioBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $musicRadioAfter=& "$musicRadioBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $musicRadioBefore -ne $musicRadioAfter) {throw "MusicRadio comparison failed: $musicRadioBefore / $musicRadioAfter"}
   $musicRadioRuns += "$musicRadioCompiler-$musicRadioOptimization-$musicRadioShape : $musicRadioAfter"
   Write-Output $musicRadioRuns[-1]
  }
 }
}
$musicRadioRuns | ConvertTo-Json | Set-Content "$musicRadioBuild/runtime-runs.json"
