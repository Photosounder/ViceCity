$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$boatRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$boatBuild=Join-Path $boatRoot 'build/audio-boat-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_boat_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Boat source audit failed'}
$boatRuns=@()
foreach($boatCompiler in @('clang','gcc')) {
 foreach($boatOptimization in @('O0','O2')) {
  $boatCpp=if($boatCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($boatShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale')) {
   $boatArgs=@("-I$boatRoot/src/audio","-I$boatRoot/src/core",'-include',"$boatBuild/$boatShape.h")
   foreach($boatUnit in @('AudioBoat','AudioMath','AudioRequests','AudioSoundQueue')) {
    & $boatCompiler -std=c11 -pedantic-errors "-$boatOptimization" @boatArgs -c "$boatRoot/src/audio/$boatUnit.c" -o "$boatBuild/$boatUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $boatUnit"}
   }
   & $boatCompiler -std=c11 -pedantic-errors "-$boatOptimization" @boatArgs -c "$boatBuild/caller.c" -o "$boatBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $boatCommon=@("$boatBuild/AudioMath.o","$boatBuild/AudioRequests.o","$boatBuild/AudioSoundQueue.o")
   & $boatCpp -std=c++17 "-$boatOptimization" @boatArgs "$boatBuild/before.cpp" @boatCommon -o "$boatBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $boatCpp -std=c++17 "-$boatOptimization" @boatArgs "$boatBuild/after.cpp" @boatCommon "$boatBuild/AudioBoat.o" "$boatBuild/caller.o" -o "$boatBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $boatBefore=& "$boatBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $boatAfter=& "$boatBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $boatBefore -ne $boatAfter) {throw "Boat comparison failed: $boatBefore / $boatAfter"}
   $boatRuns += "$boatCompiler-$boatOptimization-$boatShape : $boatAfter"
   Write-Output $boatRuns[-1]
  }
 }
}
$boatRuns | ConvertTo-Json | Set-Content "$boatBuild/runtime-runs.json"
$boatSanBuild=Join-Path $boatBuild 'sanitized'
New-Item -ItemType Directory -Path $boatSanBuild -Force | Out-Null
$boatSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$boatRoot/src/audio","-I$boatRoot/src/core",'-include',"$boatBuild/default.h")
foreach($boatSanUnit in @('AudioBoat','AudioMath','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @boatSanArgs -c "$boatRoot/src/audio/$boatSanUnit.c" -o "$boatSanBuild/$boatSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $boatSanUnit"}
}
& clang -std=c11 @boatSanArgs -c "$boatBuild/caller.c" -o "$boatSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$boatSanCommon=@("$boatSanBuild/AudioMath.o","$boatSanBuild/AudioRequests.o","$boatSanBuild/AudioSoundQueue.o")
$boatSanResults=@()
foreach($boatSanVariant in @('before','after')) {
 $boatSanExtra=if($boatSanVariant -eq 'after') {@("$boatSanBuild/AudioBoat.o","$boatSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @boatSanArgs "$boatBuild/$boatSanVariant.cpp" @boatSanCommon @boatSanExtra -o "$boatSanBuild/$boatSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $boatSanResults += & "$boatSanBuild/$boatSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $boatSanVariant"}
}
if($boatSanResults.Count -ne 2 -or $boatSanResults[0] -ne $boatSanResults[1]) {throw 'Sanitized comparison failed'}
$boatSanResults | ConvertTo-Json | Set-Content "$boatBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($boatSanResults[1])"
