$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$collisionSoundsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$collisionSoundsBuild=Join-Path $collisionSoundsRoot 'build/audio-collision-sounds-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_collision_sounds_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'CollisionSounds source audit failed'}
$collisionSoundsRuns=@()
foreach($collisionSoundsCompiler in @('clang','gcc')) {
 foreach($collisionSoundsOptimization in @('O0','O2')) {
  $collisionSoundsCpp=if($collisionSoundsCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($collisionSoundsShape in @('default','vanilla','no-reverb','no-external','ps2')) {
   $collisionSoundsArgs=@("-I$collisionSoundsRoot/src/audio","-I$collisionSoundsRoot/src/core",'-include',"$collisionSoundsBuild/$collisionSoundsShape.h")
   foreach($collisionSoundsUnit in @('AudioCollisionSounds','AudioCollisionService','AudioCollisionQueue','AudioCollisionMath','AudioMath','AudioRequests','AudioSoundQueue')) {
    & $collisionSoundsCompiler -std=c11 -pedantic-errors "-$collisionSoundsOptimization" @collisionSoundsArgs -c "$collisionSoundsRoot/src/audio/$collisionSoundsUnit.c" -o "$collisionSoundsBuild/$collisionSoundsUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $collisionSoundsUnit"}
   }
   & $collisionSoundsCompiler -std=c11 -pedantic-errors "-$collisionSoundsOptimization" @collisionSoundsArgs -c "$collisionSoundsBuild/caller.c" -o "$collisionSoundsBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $collisionSoundsCommon=@("$collisionSoundsBuild/AudioCollisionQueue.o","$collisionSoundsBuild/AudioCollisionMath.o","$collisionSoundsBuild/AudioMath.o","$collisionSoundsBuild/AudioRequests.o","$collisionSoundsBuild/AudioSoundQueue.o")
   & $collisionSoundsCpp -std=c++17 "-$collisionSoundsOptimization" @collisionSoundsArgs "$collisionSoundsBuild/before.cpp" @collisionSoundsCommon -o "$collisionSoundsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $collisionSoundsCpp -std=c++17 "-$collisionSoundsOptimization" @collisionSoundsArgs "$collisionSoundsBuild/after.cpp" @collisionSoundsCommon "$collisionSoundsBuild/AudioCollisionSounds.o" "$collisionSoundsBuild/AudioCollisionService.o" "$collisionSoundsBuild/caller.o" -o "$collisionSoundsBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $collisionSoundsBefore=& "$collisionSoundsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $collisionSoundsAfter=& "$collisionSoundsBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $collisionSoundsBefore -ne $collisionSoundsAfter) {throw "CollisionSounds comparison failed: $collisionSoundsBefore / $collisionSoundsAfter"}
   $collisionSoundsRuns += "$collisionSoundsCompiler-$collisionSoundsOptimization-$collisionSoundsShape : $collisionSoundsAfter"
   Write-Output $collisionSoundsRuns[-1]
  }
 }
}
$collisionSoundsRuns | ConvertTo-Json | Set-Content "$collisionSoundsBuild/runtime-runs.json"
$collisionSoundsSanBuild=Join-Path $collisionSoundsBuild 'sanitized'
New-Item -ItemType Directory -Path $collisionSoundsSanBuild -Force | Out-Null
$collisionSoundsSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$collisionSoundsRoot/src/audio","-I$collisionSoundsRoot/src/core",'-include',"$collisionSoundsBuild/default.h")
foreach($collisionSoundsSanUnit in @('AudioCollisionSounds','AudioCollisionService','AudioCollisionQueue','AudioCollisionMath','AudioMath','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @collisionSoundsSanArgs -c "$collisionSoundsRoot/src/audio/$collisionSoundsSanUnit.c" -o "$collisionSoundsSanBuild/$collisionSoundsSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $collisionSoundsSanUnit"}
}
& clang -std=c11 @collisionSoundsSanArgs -c "$collisionSoundsBuild/caller.c" -o "$collisionSoundsSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$collisionSoundsSanCommon=@("$collisionSoundsSanBuild/AudioCollisionQueue.o","$collisionSoundsSanBuild/AudioCollisionMath.o","$collisionSoundsSanBuild/AudioMath.o","$collisionSoundsSanBuild/AudioRequests.o","$collisionSoundsSanBuild/AudioSoundQueue.o")
$collisionSoundsSanResults=@()
foreach($collisionSoundsSanVariant in @('before','after')) {
 $collisionSoundsSanExtra=if($collisionSoundsSanVariant -eq 'after') {@("$collisionSoundsSanBuild/AudioCollisionSounds.o","$collisionSoundsSanBuild/AudioCollisionService.o","$collisionSoundsSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @collisionSoundsSanArgs "$collisionSoundsBuild/$collisionSoundsSanVariant.cpp" @collisionSoundsSanCommon @collisionSoundsSanExtra -o "$collisionSoundsSanBuild/$collisionSoundsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $collisionSoundsSanResults += & "$collisionSoundsSanBuild/$collisionSoundsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $collisionSoundsSanVariant"}
}
if($collisionSoundsSanResults.Count -ne 2 -or $collisionSoundsSanResults[0] -ne $collisionSoundsSanResults[1]) {throw 'Sanitized comparison failed'}
$collisionSoundsSanResults | ConvertTo-Json | Set-Content "$collisionSoundsBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($collisionSoundsSanResults[1])"
