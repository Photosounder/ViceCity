$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicDisplayRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicDisplayBuild=Join-Path $musicDisplayRoot 'build/audio-music-display-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_music_display_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'MusicDisplay source audit failed'}
$musicDisplayRuns=@()
foreach($musicDisplayCompiler in @('clang','gcc')) {
 foreach($musicDisplayOptimization in @('O0','O2')) {
  $musicDisplayCpp=if($musicDisplayCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicDisplayShape in @('default','vanilla','no-off-text','no-previous-station','ps2','no-aspect','force-pc')) {
   $musicDisplayArgs=@("-I$musicDisplayRoot/src/audio","-I$musicDisplayRoot/src/core",'-include',"$musicDisplayBuild/$musicDisplayShape.h")
   foreach($musicDisplayUnit in @('MusicDisplay')) {
    & $musicDisplayCompiler -std=c11 -pedantic-errors "-$musicDisplayOptimization" @musicDisplayArgs -c "$musicDisplayRoot/src/audio/$musicDisplayUnit.c" -o "$musicDisplayBuild/$musicDisplayUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicDisplayUnit"}
   }
   & $musicDisplayCompiler -std=c11 -pedantic-errors "-$musicDisplayOptimization" @musicDisplayArgs -c "$musicDisplayBuild/caller.c" -o "$musicDisplayBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicDisplayCompiler -std=c11 -O2 -c "$musicDisplayBuild/fast_hash.c" -o "$musicDisplayBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicDisplayCommon=@("$musicDisplayBuild/fast_hash.o")
   & $musicDisplayCpp -std=c++17 "-$musicDisplayOptimization" @musicDisplayArgs "$musicDisplayBuild/before.cpp" @musicDisplayCommon -o "$musicDisplayBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicDisplayCpp -std=c++17 "-$musicDisplayOptimization" @musicDisplayArgs "$musicDisplayBuild/after.cpp" @musicDisplayCommon "$musicDisplayBuild/MusicDisplay.o" "$musicDisplayBuild/caller.o" -o "$musicDisplayBuild/after.exe"
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
