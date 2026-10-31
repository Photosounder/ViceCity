$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicRadioRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicRadioBuild=Join-Path $musicRadioRoot 'build/audio-music-radio-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_music_radio_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'MusicRadio source audit failed'}
$musicRadioRuns=@()
foreach($musicRadioCompiler in @('clang','gcc')) {
 foreach($musicRadioOptimization in @('O0','O2')) {
  $musicRadioCpp=if($musicRadioCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicRadioShape in @('default','vanilla','no-external','ps2','no-previous-station')) {
   $musicRadioArgs=@("-I$musicRadioRoot/src/audio","-I$musicRadioRoot/src/core",'-include',"$musicRadioBuild/$musicRadioShape.h")
   foreach($musicRadioUnit in @('MusicRadio','MusicState')) {
    & $musicRadioCompiler -std=c11 -pedantic-errors "-$musicRadioOptimization" @musicRadioArgs -c "$musicRadioRoot/src/audio/$musicRadioUnit.c" -o "$musicRadioBuild/$musicRadioUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicRadioUnit"}
   }
   & $musicRadioCompiler -std=c11 -pedantic-errors "-$musicRadioOptimization" @musicRadioArgs -c "$musicRadioBuild/caller.c" -o "$musicRadioBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicRadioCompiler -std=c11 -O2 -c "$musicRadioBuild/fast_hash.c" -o "$musicRadioBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicRadioCommon=@("$musicRadioBuild/fast_hash.o","$musicRadioBuild/MusicState.o")
   & $musicRadioCpp -std=c++17 "-$musicRadioOptimization" @musicRadioArgs "$musicRadioBuild/before.cpp" @musicRadioCommon -o "$musicRadioBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicRadioCpp -std=c++17 "-$musicRadioOptimization" @musicRadioArgs "$musicRadioBuild/after.cpp" @musicRadioCommon "$musicRadioBuild/MusicRadio.o" "$musicRadioBuild/caller.o" -o "$musicRadioBuild/after.exe"
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
