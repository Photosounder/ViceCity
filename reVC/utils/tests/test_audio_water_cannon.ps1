$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$waterCannonRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$waterCannonBuild=Join-Path $waterCannonRoot 'build/audio-water-cannon-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_water_cannon_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'WaterCannon source audit failed'}
$waterCannonRuns=@()
foreach($waterCannonCompiler in @('clang','gcc')) {
 foreach($waterCannonOptimization in @('O0','O2')) {
  $waterCannonCpp=if($waterCannonCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($waterCannonShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale','no-reflections')) {
   $waterCannonArgs=@("-I$waterCannonRoot/src/audio","-I$waterCannonRoot/src/core",'-include',"$waterCannonBuild/$waterCannonShape.h")
   foreach($waterCannonUnit in @('AudioWaterCannon','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
    & $waterCannonCompiler -std=c11 -pedantic-errors "-$waterCannonOptimization" @waterCannonArgs -c "$waterCannonRoot/src/audio/$waterCannonUnit.c" $(if($waterCannonUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestWaterCannonRealSubmit'}) -o "$waterCannonBuild/$waterCannonUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $waterCannonUnit"}
   }
   & $waterCannonCompiler -std=c11 -pedantic-errors "-$waterCannonOptimization" @waterCannonArgs -c "$waterCannonBuild/caller.c" -o "$waterCannonBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $waterCannonCompiler -std=c11 -O2 -c "$waterCannonBuild/fast_hash.c" -o "$waterCannonBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $waterCannonCommon=@("$waterCannonBuild/fast_hash.o","$waterCannonBuild/AudioMath.o","$waterCannonBuild/AudioGeometry.o","$waterCannonBuild/AudioRequests.o","$waterCannonBuild/AudioSoundQueue.o")
   & $waterCannonCpp -std=c++17 "-$waterCannonOptimization" @waterCannonArgs "$waterCannonBuild/before.cpp" @waterCannonCommon -o "$waterCannonBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $waterCannonCpp -std=c++17 "-$waterCannonOptimization" @waterCannonArgs "$waterCannonBuild/after.cpp" @waterCannonCommon "$waterCannonBuild/AudioWaterCannon.o" "$waterCannonBuild/caller.o" -o "$waterCannonBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $waterCannonBefore=& "$waterCannonBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $waterCannonAfter=& "$waterCannonBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $waterCannonBefore -ne $waterCannonAfter) {throw "WaterCannon comparison failed: $waterCannonBefore / $waterCannonAfter"}
   $waterCannonRuns += "$waterCannonCompiler-$waterCannonOptimization-$waterCannonShape : $waterCannonAfter"
   Write-Output $waterCannonRuns[-1]
  }
 }
}
$waterCannonRuns | ConvertTo-Json | Set-Content "$waterCannonBuild/runtime-runs.json"
$waterCannonSanBuild=Join-Path $waterCannonBuild 'sanitized'
New-Item -ItemType Directory -Path $waterCannonSanBuild -Force | Out-Null
$waterCannonSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$waterCannonRoot/src/audio","-I$waterCannonRoot/src/core",'-include',"$waterCannonBuild/default.h")
foreach($waterCannonSanUnit in @('AudioWaterCannon','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @waterCannonSanArgs -c "$waterCannonRoot/src/audio/$waterCannonSanUnit.c" $(if($waterCannonSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestWaterCannonRealSubmit'}) -o "$waterCannonSanBuild/$waterCannonSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $waterCannonSanUnit"}
}
& clang -std=c11 @waterCannonSanArgs -c "$waterCannonBuild/caller.c" -o "$waterCannonSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @waterCannonSanArgs -c "$waterCannonBuild/fast_hash.c" -o "$waterCannonSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$waterCannonSanCommon=@("$waterCannonSanBuild/fast_hash.o","$waterCannonSanBuild/AudioMath.o","$waterCannonSanBuild/AudioGeometry.o","$waterCannonSanBuild/AudioRequests.o","$waterCannonSanBuild/AudioSoundQueue.o")
$waterCannonSanResults=@()
foreach($waterCannonSanVariant in @('before','after')) {
 $waterCannonSanExtra=if($waterCannonSanVariant -eq 'after') {@("$waterCannonSanBuild/AudioWaterCannon.o","$waterCannonSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @waterCannonSanArgs "$waterCannonBuild/$waterCannonSanVariant.cpp" @waterCannonSanCommon @waterCannonSanExtra -o "$waterCannonSanBuild/$waterCannonSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $waterCannonSanResults += & "$waterCannonSanBuild/$waterCannonSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $waterCannonSanVariant"}
}
if($waterCannonSanResults.Count -ne 2 -or $waterCannonSanResults[0] -ne $waterCannonSanResults[1]) {throw 'Sanitized comparison failed'}
$waterCannonSanResults | ConvertTo-Json | Set-Content "$waterCannonBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($waterCannonSanResults[1])"
