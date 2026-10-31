$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$musicReloadRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$musicReloadBuild=Join-Path $musicReloadRoot 'build/audio-music-reload-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_music_reload_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'MusicReload source audit failed'}
$musicReloadRuns=@()
foreach($musicReloadCompiler in @('clang','gcc')) {
 foreach($musicReloadOptimization in @('O0','O2')) {
  $musicReloadCpp=if($musicReloadCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($musicReloadShape in @('default','vanilla','no-external','ps2')) {
   $musicReloadArgs=@("-I$musicReloadRoot/src/audio","-I$musicReloadRoot/src/core",'-include',"$musicReloadBuild/$musicReloadShape.h")
   foreach($musicReloadUnit in @('MusicReload','MusicSavedData')) {
    & $musicReloadCompiler -std=c11 -pedantic-errors "-$musicReloadOptimization" @musicReloadArgs -c "$musicReloadRoot/src/audio/$musicReloadUnit.c" -o "$musicReloadBuild/$musicReloadUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $musicReloadUnit"}
   }
   & $musicReloadCompiler -std=c11 -pedantic-errors "-$musicReloadOptimization" @musicReloadArgs -c "$musicReloadBuild/caller.c" -o "$musicReloadBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $musicReloadCompiler -std=c11 -O2 -c "$musicReloadBuild/fast_hash.c" -o "$musicReloadBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $musicReloadCommon=@("$musicReloadBuild/fast_hash.o")
   & $musicReloadCpp -std=c++17 "-$musicReloadOptimization" @musicReloadArgs "$musicReloadBuild/before.cpp" @musicReloadCommon -o "$musicReloadBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $musicReloadCpp -std=c++17 "-$musicReloadOptimization" @musicReloadArgs "$musicReloadBuild/after.cpp" @musicReloadCommon "$musicReloadBuild/MusicReload.o" "$musicReloadBuild/MusicSavedData.o" "$musicReloadBuild/caller.o" -o "$musicReloadBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $musicReloadBefore=& "$musicReloadBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $musicReloadAfter=& "$musicReloadBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $musicReloadBefore -ne $musicReloadAfter) {throw "MusicReload comparison failed: $musicReloadBefore / $musicReloadAfter"}
   $musicReloadRuns += "$musicReloadCompiler-$musicReloadOptimization-$musicReloadShape : $musicReloadAfter"
   Write-Output $musicReloadRuns[-1]
  }
 }
}
$musicReloadRuns | ConvertTo-Json | Set-Content "$musicReloadBuild/runtime-runs.json"
