$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$collisionServiceRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$collisionServiceBuild=Join-Path $collisionServiceRoot 'build/audio-collision-service-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_collision_service_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'CollisionService source audit failed'}
$collisionServiceRuns=@()
foreach($collisionServiceCompiler in @('clang','gcc')) {
 foreach($collisionServiceOptimization in @('O0','O2')) {
  $collisionServiceCpp=if($collisionServiceCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($collisionServiceShape in @('default','vanilla','no-reverb','no-external','ps2')) {
   $collisionServiceArgs=@("-I$collisionServiceRoot/src/audio","-I$collisionServiceRoot/src/core",'-include',"$collisionServiceBuild/$collisionServiceShape.h")
   foreach($collisionServiceUnit in @('AudioCollisionService','AudioCollisionQueue')) {
    & $collisionServiceCompiler -std=c11 -pedantic-errors "-$collisionServiceOptimization" @collisionServiceArgs -c "$collisionServiceRoot/src/audio/$collisionServiceUnit.c" -o "$collisionServiceBuild/$collisionServiceUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $collisionServiceUnit"}
   }
   & $collisionServiceCompiler -std=c11 -pedantic-errors "-$collisionServiceOptimization" @collisionServiceArgs -c "$collisionServiceBuild/caller.c" -o "$collisionServiceBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $collisionServiceCommon=@("$collisionServiceBuild/AudioCollisionQueue.o")
   & $collisionServiceCpp -std=c++17 "-$collisionServiceOptimization" @collisionServiceArgs "$collisionServiceBuild/before.cpp" @collisionServiceCommon -o "$collisionServiceBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $collisionServiceCpp -std=c++17 "-$collisionServiceOptimization" @collisionServiceArgs "$collisionServiceBuild/after.cpp" @collisionServiceCommon "$collisionServiceBuild/AudioCollisionService.o" "$collisionServiceBuild/caller.o" -o "$collisionServiceBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $collisionServiceBefore=& "$collisionServiceBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $collisionServiceAfter=& "$collisionServiceBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $collisionServiceBefore -ne $collisionServiceAfter) {throw "CollisionService comparison failed: $collisionServiceBefore / $collisionServiceAfter"}
   $collisionServiceRuns += "$collisionServiceCompiler-$collisionServiceOptimization-$collisionServiceShape : $collisionServiceAfter"
   Write-Output $collisionServiceRuns[-1]
  }
 }
}
$collisionServiceRuns | ConvertTo-Json | Set-Content "$collisionServiceBuild/runtime-runs.json"
$collisionServiceSanBuild=Join-Path $collisionServiceBuild 'sanitized'
New-Item -ItemType Directory -Path $collisionServiceSanBuild -Force | Out-Null
$collisionServiceSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$collisionServiceRoot/src/audio","-I$collisionServiceRoot/src/core",'-include',"$collisionServiceBuild/default.h")
foreach($collisionServiceSanUnit in @('AudioCollisionService','AudioCollisionQueue')) {
 & clang -std=c11 @collisionServiceSanArgs -c "$collisionServiceRoot/src/audio/$collisionServiceSanUnit.c" -o "$collisionServiceSanBuild/$collisionServiceSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $collisionServiceSanUnit"}
}
& clang -std=c11 @collisionServiceSanArgs -c "$collisionServiceBuild/caller.c" -o "$collisionServiceSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$collisionServiceSanCommon=@("$collisionServiceSanBuild/AudioCollisionQueue.o")
$collisionServiceSanResults=@()
foreach($collisionServiceSanVariant in @('before','after')) {
 $collisionServiceSanExtra=if($collisionServiceSanVariant -eq 'after') {@("$collisionServiceSanBuild/AudioCollisionService.o","$collisionServiceSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @collisionServiceSanArgs "$collisionServiceBuild/$collisionServiceSanVariant.cpp" @collisionServiceSanCommon @collisionServiceSanExtra -o "$collisionServiceSanBuild/$collisionServiceSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $collisionServiceSanResults += & "$collisionServiceSanBuild/$collisionServiceSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $collisionServiceSanVariant"}
}
if($collisionServiceSanResults.Count -ne 2 -or $collisionServiceSanResults[0] -ne $collisionServiceSanResults[1]) {throw 'Sanitized comparison failed'}
$collisionServiceSanResults | ConvertTo-Json | Set-Content "$collisionServiceBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($collisionServiceSanResults[1])"
