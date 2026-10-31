$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$planeRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$planeBuild=Join-Path $planeRoot 'build/audio-plane-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_plane_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Plane source audit failed'}
$planeRuns=@()
foreach($planeCompiler in @('clang','gcc')) {
 foreach($planeOptimization in @('O0','O2')) {
  $planeCpp=if($planeCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($planeShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale')) {
   $planeArgs=@("-I$planeRoot/src/audio","-I$planeRoot/src/core",'-include',"$planeBuild/$planeShape.h")
   foreach($planeUnit in @('AudioPlane','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
    & $planeCompiler -std=c11 -pedantic-errors "-$planeOptimization" @planeArgs -c "$planeRoot/src/audio/$planeUnit.c" $(if($planeUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestPlaneRealSubmit'}) -o "$planeBuild/$planeUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $planeUnit"}
   }
   & $planeCompiler -std=c11 -pedantic-errors "-$planeOptimization" @planeArgs -c "$planeBuild/caller.c" -o "$planeBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $planeCompiler -std=c11 -O2 -c "$planeBuild/fast_hash.c" -o "$planeBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $planeCommon=@("$planeBuild/fast_hash.o","$planeBuild/AudioMath.o","$planeBuild/AudioGeometry.o","$planeBuild/AudioRequests.o","$planeBuild/AudioSoundQueue.o")
   & $planeCpp -std=c++17 "-$planeOptimization" @planeArgs "$planeBuild/before.cpp" @planeCommon -o "$planeBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $planeCpp -std=c++17 "-$planeOptimization" @planeArgs "$planeBuild/after.cpp" @planeCommon "$planeBuild/AudioPlane.o" "$planeBuild/caller.o" -o "$planeBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $planeBefore=& "$planeBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $planeAfter=& "$planeBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $planeBefore -ne $planeAfter) {throw "Plane comparison failed: $planeBefore / $planeAfter"}
   $planeRuns += "$planeCompiler-$planeOptimization-$planeShape : $planeAfter"
   Write-Output $planeRuns[-1]
  }
 }
}
$planeRuns | ConvertTo-Json | Set-Content "$planeBuild/runtime-runs.json"
$planeSanBuild=Join-Path $planeBuild 'sanitized'
New-Item -ItemType Directory -Path $planeSanBuild -Force | Out-Null
$planeSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$planeRoot/src/audio","-I$planeRoot/src/core",'-include',"$planeBuild/default.h")
foreach($planeSanUnit in @('AudioPlane','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @planeSanArgs -c "$planeRoot/src/audio/$planeSanUnit.c" $(if($planeSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestPlaneRealSubmit'}) -o "$planeSanBuild/$planeSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $planeSanUnit"}
}
& clang -std=c11 @planeSanArgs -c "$planeBuild/caller.c" -o "$planeSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @planeSanArgs -c "$planeBuild/fast_hash.c" -o "$planeSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$planeSanCommon=@("$planeSanBuild/fast_hash.o","$planeSanBuild/AudioMath.o","$planeSanBuild/AudioGeometry.o","$planeSanBuild/AudioRequests.o","$planeSanBuild/AudioSoundQueue.o")
$planeSanResults=@()
foreach($planeSanVariant in @('before','after')) {
 $planeSanExtra=if($planeSanVariant -eq 'after') {@("$planeSanBuild/AudioPlane.o","$planeSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @planeSanArgs "$planeBuild/$planeSanVariant.cpp" @planeSanCommon @planeSanExtra -o "$planeSanBuild/$planeSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $planeSanResults += & "$planeSanBuild/$planeSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $planeSanVariant"}
}
if($planeSanResults.Count -ne 2 -or $planeSanResults[0] -ne $planeSanResults[1]) {throw 'Sanitized comparison failed'}
$planeSanResults | ConvertTo-Json | Set-Content "$planeBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($planeSanResults[1])"
