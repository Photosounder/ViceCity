$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$effectsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$effectsBuild=Join-Path $effectsRoot 'build/audio-effects-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_effects_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Effects source audit failed'}
$effectsRuns=@()
foreach($effectsCompiler in @('clang','gcc')) {
 foreach($effectsOptimization in @('O0','O2')) {
  $effectsCpp=if($effectsCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($effectsShape in @('default','vanilla','no-oal','no-external','no-reverb','ps2')) {
   $effectsArgs=@("-I$effectsRoot/src/audio","-I$effectsRoot/src/core",'-include',"$effectsBuild/$effectsShape.h")
   foreach($effectsUnit in @('AudioEffects','AudioEntities','AudioSoundReset','AudioSoundVolume','AudioMath')) {
    & $effectsCompiler -std=c11 -pedantic-errors "-$effectsOptimization" @effectsArgs -c "$effectsRoot/src/audio/$effectsUnit.c" -o "$effectsBuild/$effectsUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $effectsUnit"}
   }
   & $effectsCompiler -std=c11 -pedantic-errors "-$effectsOptimization" @effectsArgs -c "$effectsBuild/script-reset.c" -o "$effectsBuild/script-reset.o"
   if($LASTEXITCODE -ne 0) {throw 'C script reset unit compile failed'}
   & $effectsCompiler -std=c11 -pedantic-errors "-$effectsOptimization" @effectsArgs -c "$effectsBuild/caller.c" -o "$effectsBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $effectsCommon=@("$effectsBuild/script-reset.o","$effectsBuild/AudioEntities.o","$effectsBuild/AudioSoundReset.o","$effectsBuild/AudioSoundVolume.o","$effectsBuild/AudioMath.o")
   & $effectsCpp -std=c++17 "-$effectsOptimization" @effectsArgs "$effectsBuild/before.cpp" @effectsCommon -o "$effectsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $effectsCpp -std=c++17 "-$effectsOptimization" @effectsArgs "$effectsBuild/after.cpp" @effectsCommon "$effectsBuild/AudioEffects.o" "$effectsBuild/caller.o" -o "$effectsBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $effectsBefore=& "$effectsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $effectsAfter=& "$effectsBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $effectsBefore -ne $effectsAfter) {throw "Effects comparison failed: $effectsBefore / $effectsAfter"}
   $effectsRuns += "$effectsCompiler-$effectsOptimization-$effectsShape : $effectsAfter"
   Write-Output $effectsRuns[-1]
  }
 }
}
$effectsRuns | ConvertTo-Json | Set-Content "$effectsBuild/runtime-runs.json"
