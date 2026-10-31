$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$garageHydrantRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$garageHydrantBuild=Join-Path $garageHydrantRoot 'build/audio-garage-hydrant-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_garage_hydrant_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'GarageHydrant source audit failed'}
$garageHydrantRuns=@()
foreach($garageHydrantCompiler in @('clang','gcc')) {
 foreach($garageHydrantOptimization in @('O0','O2')) {
  $garageHydrantCpp=if($garageHydrantCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($garageHydrantShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale','no-reflections')) {
   $garageHydrantArgs=@("-I$garageHydrantRoot/src/audio","-I$garageHydrantRoot/src/core",'-include',"$garageHydrantBuild/$garageHydrantShape.h")
   foreach($garageHydrantUnit in @('AudioGarageHydrant','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
    & $garageHydrantCompiler -std=c11 -pedantic-errors "-$garageHydrantOptimization" @garageHydrantArgs -c "$garageHydrantRoot/src/audio/$garageHydrantUnit.c" $(if($garageHydrantUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestGarageHydrantRealSubmit'}) -o "$garageHydrantBuild/$garageHydrantUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $garageHydrantUnit"}
   }
   & $garageHydrantCompiler -std=c11 -pedantic-errors "-$garageHydrantOptimization" @garageHydrantArgs -c "$garageHydrantBuild/caller.c" -o "$garageHydrantBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $garageHydrantCompiler -std=c11 -O2 -c "$garageHydrantBuild/fast_hash.c" -o "$garageHydrantBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $garageHydrantCommon=@("$garageHydrantBuild/fast_hash.o","$garageHydrantBuild/AudioMath.o","$garageHydrantBuild/AudioGeometry.o","$garageHydrantBuild/AudioRequests.o","$garageHydrantBuild/AudioSoundQueue.o")
   & $garageHydrantCpp -std=c++17 "-$garageHydrantOptimization" @garageHydrantArgs "$garageHydrantBuild/before.cpp" @garageHydrantCommon -o "$garageHydrantBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $garageHydrantCpp -std=c++17 "-$garageHydrantOptimization" @garageHydrantArgs "$garageHydrantBuild/after.cpp" @garageHydrantCommon "$garageHydrantBuild/AudioGarageHydrant.o" "$garageHydrantBuild/caller.o" -o "$garageHydrantBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $garageHydrantBefore=& "$garageHydrantBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $garageHydrantAfter=& "$garageHydrantBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $garageHydrantBefore -ne $garageHydrantAfter) {throw "GarageHydrant comparison failed: $garageHydrantBefore / $garageHydrantAfter"}
   $garageHydrantRuns += "$garageHydrantCompiler-$garageHydrantOptimization-$garageHydrantShape : $garageHydrantAfter"
   Write-Output $garageHydrantRuns[-1]
  }
 }
}
$garageHydrantRuns | ConvertTo-Json | Set-Content "$garageHydrantBuild/runtime-runs.json"
$garageHydrantSanBuild=Join-Path $garageHydrantBuild 'sanitized'
New-Item -ItemType Directory -Path $garageHydrantSanBuild -Force | Out-Null
$garageHydrantSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$garageHydrantRoot/src/audio","-I$garageHydrantRoot/src/core",'-include',"$garageHydrantBuild/default.h")
foreach($garageHydrantSanUnit in @('AudioGarageHydrant','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @garageHydrantSanArgs -c "$garageHydrantRoot/src/audio/$garageHydrantSanUnit.c" $(if($garageHydrantSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestGarageHydrantRealSubmit'}) -o "$garageHydrantSanBuild/$garageHydrantSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $garageHydrantSanUnit"}
}
& clang -std=c11 @garageHydrantSanArgs -c "$garageHydrantBuild/caller.c" -o "$garageHydrantSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @garageHydrantSanArgs -c "$garageHydrantBuild/fast_hash.c" -o "$garageHydrantSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$garageHydrantSanCommon=@("$garageHydrantSanBuild/fast_hash.o","$garageHydrantSanBuild/AudioMath.o","$garageHydrantSanBuild/AudioGeometry.o","$garageHydrantSanBuild/AudioRequests.o","$garageHydrantSanBuild/AudioSoundQueue.o")
$garageHydrantSanResults=@()
foreach($garageHydrantSanVariant in @('before','after')) {
 $garageHydrantSanExtra=if($garageHydrantSanVariant -eq 'after') {@("$garageHydrantSanBuild/AudioGarageHydrant.o","$garageHydrantSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @garageHydrantSanArgs "$garageHydrantBuild/$garageHydrantSanVariant.cpp" @garageHydrantSanCommon @garageHydrantSanExtra -o "$garageHydrantSanBuild/$garageHydrantSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $garageHydrantSanResults += & "$garageHydrantSanBuild/$garageHydrantSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $garageHydrantSanVariant"}
}
if($garageHydrantSanResults.Count -ne 2 -or $garageHydrantSanResults[0] -ne $garageHydrantSanResults[1]) {throw 'Sanitized comparison failed'}
$garageHydrantSanResults | ConvertTo-Json | Set-Content "$garageHydrantBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($garageHydrantSanResults[1])"
