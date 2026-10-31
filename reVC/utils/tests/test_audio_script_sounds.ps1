$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$scriptSoundsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$scriptSoundsBuild=Join-Path $scriptSoundsRoot 'build/audio-script-sounds-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_script_sounds_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'ScriptSounds source audit failed'}
$scriptSoundsRuns=@()
foreach($scriptSoundsCompiler in @('clang','gcc')) {
 foreach($scriptSoundsOptimization in @('O0','O2')) {
  $scriptSoundsCpp=if($scriptSoundsCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($scriptSoundsShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale','no-reflections')) {
   $scriptSoundsArgs=@("-I$scriptSoundsRoot/src/audio","-I$scriptSoundsRoot/src/core",'-include',"$scriptSoundsBuild/$scriptSoundsShape.h")
   foreach($scriptSoundsUnit in @('AudioScriptSounds','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
    & $scriptSoundsCompiler -std=c11 -pedantic-errors "-$scriptSoundsOptimization" @scriptSoundsArgs -c "$scriptSoundsRoot/src/audio/$scriptSoundsUnit.c" $(if($scriptSoundsUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestScriptSoundsRealSubmit'}) -o "$scriptSoundsBuild/$scriptSoundsUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $scriptSoundsUnit"}
   }
   & $scriptSoundsCompiler -std=c11 -pedantic-errors "-$scriptSoundsOptimization" @scriptSoundsArgs -c "$scriptSoundsBuild/caller.c" -o "$scriptSoundsBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $scriptSoundsCompiler -std=c11 -pedantic-errors "-$scriptSoundsOptimization" @scriptSoundsArgs -c "$scriptSoundsBuild/music-owner.c" -o "$scriptSoundsBuild/music-owner.o"
   if($LASTEXITCODE -ne 0) {throw 'C music record compile failed'}
   & $scriptSoundsCompiler -std=c11 -O2 -c "$scriptSoundsBuild/fast_hash.c" -o "$scriptSoundsBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $scriptSoundsCommon=@("$scriptSoundsBuild/fast_hash.o","$scriptSoundsBuild/AudioMath.o","$scriptSoundsBuild/AudioGeometry.o","$scriptSoundsBuild/AudioRequests.o","$scriptSoundsBuild/AudioSoundQueue.o")
   & $scriptSoundsCpp -std=c++17 "-$scriptSoundsOptimization" @scriptSoundsArgs "$scriptSoundsBuild/before.cpp" @scriptSoundsCommon -o "$scriptSoundsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $scriptSoundsCpp -std=c++17 "-$scriptSoundsOptimization" @scriptSoundsArgs "$scriptSoundsBuild/after.cpp" @scriptSoundsCommon "$scriptSoundsBuild/AudioScriptSounds.o" "$scriptSoundsBuild/caller.o" "$scriptSoundsBuild/music-owner.o" -o "$scriptSoundsBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $scriptSoundsBefore=& "$scriptSoundsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $scriptSoundsAfter=& "$scriptSoundsBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $scriptSoundsBefore -ne $scriptSoundsAfter) {throw "ScriptSounds comparison failed: $scriptSoundsBefore / $scriptSoundsAfter"}
   $scriptSoundsRuns += "$scriptSoundsCompiler-$scriptSoundsOptimization-$scriptSoundsShape : $scriptSoundsAfter"
   Write-Output $scriptSoundsRuns[-1]
  }
 }
}
$scriptSoundsRuns | ConvertTo-Json | Set-Content "$scriptSoundsBuild/runtime-runs.json"
$scriptSoundsSanBuild=Join-Path $scriptSoundsBuild 'sanitized'
New-Item -ItemType Directory -Path $scriptSoundsSanBuild -Force | Out-Null
$scriptSoundsSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$scriptSoundsRoot/src/audio","-I$scriptSoundsRoot/src/core",'-include',"$scriptSoundsBuild/default.h")
foreach($scriptSoundsSanUnit in @('AudioScriptSounds','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @scriptSoundsSanArgs -c "$scriptSoundsRoot/src/audio/$scriptSoundsSanUnit.c" $(if($scriptSoundsSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestScriptSoundsRealSubmit'}) -o "$scriptSoundsSanBuild/$scriptSoundsSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $scriptSoundsSanUnit"}
}
& clang -std=c11 @scriptSoundsSanArgs -c "$scriptSoundsBuild/caller.c" -o "$scriptSoundsSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @scriptSoundsSanArgs -c "$scriptSoundsBuild/music-owner.c" -o "$scriptSoundsSanBuild/music-owner.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized C music record compile failed'}
& clang -std=c11 @scriptSoundsSanArgs -c "$scriptSoundsBuild/fast_hash.c" -o "$scriptSoundsSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$scriptSoundsSanCommon=@("$scriptSoundsSanBuild/fast_hash.o","$scriptSoundsSanBuild/AudioMath.o","$scriptSoundsSanBuild/AudioGeometry.o","$scriptSoundsSanBuild/AudioRequests.o","$scriptSoundsSanBuild/AudioSoundQueue.o")
$scriptSoundsSanResults=@()
foreach($scriptSoundsSanVariant in @('before','after')) {
 $scriptSoundsSanExtra=if($scriptSoundsSanVariant -eq 'after') {@("$scriptSoundsSanBuild/AudioScriptSounds.o","$scriptSoundsSanBuild/caller.o","$scriptSoundsSanBuild/music-owner.o")} else {@()}
 & clang++ -std=c++17 @scriptSoundsSanArgs "$scriptSoundsBuild/$scriptSoundsSanVariant.cpp" @scriptSoundsSanCommon @scriptSoundsSanExtra -o "$scriptSoundsSanBuild/$scriptSoundsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $scriptSoundsSanResults += & "$scriptSoundsSanBuild/$scriptSoundsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $scriptSoundsSanVariant"}
}
if($scriptSoundsSanResults.Count -ne 2 -or $scriptSoundsSanResults[0] -ne $scriptSoundsSanResults[1]) {throw 'Sanitized comparison failed'}
$scriptSoundsSanResults | ConvertTo-Json | Set-Content "$scriptSoundsBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($scriptSoundsSanResults[1])"
