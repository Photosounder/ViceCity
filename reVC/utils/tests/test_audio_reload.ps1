$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$reloadRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$reloadBuild=Join-Path $reloadRoot 'build/audio-reload-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_reload_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Reload source audit failed'}
$reloadRuns=@()
foreach($reloadCompiler in @('clang','gcc')) {
 foreach($reloadOptimization in @('O0','O2')) {
  $reloadCpp=if($reloadCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($reloadShape in @('default','no-oal','no-external','ps2')) {
   $reloadArgs=@("-I$reloadRoot/src/audio","-I$reloadRoot/src/core",'-include',"$reloadBuild/$reloadShape.h")
   foreach($reloadUnit in @('AudioReload','AudioEntities','AudioSoundReset','AudioMissionState','AudioMissionPosition')) {
    & $reloadCompiler -std=c11 -pedantic-errors "-$reloadOptimization" @reloadArgs -c "$reloadRoot/src/audio/$reloadUnit.c" -o "$reloadBuild/$reloadUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $reloadUnit"}
   }
   foreach($reloadUnit in @('caller','script-reset')) {
    & $reloadCompiler -std=c11 -pedantic-errors "-$reloadOptimization" @reloadArgs -c "$reloadBuild/$reloadUnit.c" -o "$reloadBuild/$reloadUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C fixture compile failed: $reloadUnit"}
   }
   $reloadCommon=@("$reloadBuild/AudioEntities.o","$reloadBuild/AudioSoundReset.o","$reloadBuild/AudioMissionState.o","$reloadBuild/AudioMissionPosition.o","$reloadBuild/script-reset.o")
   & $reloadCpp -std=c++17 "-$reloadOptimization" @reloadArgs "$reloadBuild/before.cpp" @reloadCommon -o "$reloadBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $reloadCpp -std=c++17 "-$reloadOptimization" @reloadArgs "$reloadBuild/after.cpp" @reloadCommon "$reloadBuild/AudioReload.o" "$reloadBuild/caller.o" -o "$reloadBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $reloadBefore=& "$reloadBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $reloadAfter=& "$reloadBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $reloadBefore -ne $reloadAfter) {throw "Reload comparison failed: $reloadBefore / $reloadAfter"}
   $reloadRuns += "$reloadCompiler-$reloadOptimization-$reloadShape : $reloadAfter"
   Write-Output $reloadRuns[-1]
  }
 }
}
$reloadRuns | ConvertTo-Json | Set-Content "$reloadBuild/runtime-runs.json"
