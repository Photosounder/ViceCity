$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$pedRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$pedBuild=Join-Path $pedRoot 'build/audio-ped-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_ped_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Ped source audit failed'}
$pedRuns=@()
foreach($pedCompiler in @('clang','gcc')) {
 foreach($pedOptimization in @('O0','O2')) {
  $pedCpp=if($pedCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($pedShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale')) {
   $pedArgs=@("-I$pedRoot/src/audio","-I$pedRoot/src/core",'-include',"$pedBuild/$pedShape.h")
   foreach($pedUnit in @('AudioPed','AudioGeometry')) {
    & $pedCompiler -std=c11 -pedantic-errors "-$pedOptimization" @pedArgs -c "$pedRoot/src/audio/$pedUnit.c" $(if($pedUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestPedRealSubmit'}) -o "$pedBuild/$pedUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $pedUnit"}
   }
   & $pedCompiler -std=c11 -pedantic-errors "-$pedOptimization" @pedArgs -c "$pedBuild/caller.c" -o "$pedBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $pedCompiler -std=c11 -O2 -c "$pedBuild/fast_hash.c" -o "$pedBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $pedCommon=@("$pedBuild/fast_hash.o","$pedBuild/AudioGeometry.o")
   & $pedCpp -std=c++17 "-$pedOptimization" @pedArgs "$pedBuild/before.cpp" @pedCommon -o "$pedBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $pedCpp -std=c++17 "-$pedOptimization" @pedArgs "$pedBuild/after.cpp" @pedCommon "$pedBuild/AudioPed.o" "$pedBuild/caller.o" -o "$pedBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $pedBefore=& "$pedBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $pedAfter=& "$pedBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $pedBefore -ne $pedAfter) {throw "Ped comparison failed: $pedBefore / $pedAfter"}
   $pedRuns += "$pedCompiler-$pedOptimization-$pedShape : $pedAfter"
   Write-Output $pedRuns[-1]
  }
 }
}
$pedRuns | ConvertTo-Json | Set-Content "$pedBuild/runtime-runs.json"
$pedSanBuild=Join-Path $pedBuild 'sanitized'
New-Item -ItemType Directory -Path $pedSanBuild -Force | Out-Null
$pedSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$pedRoot/src/audio","-I$pedRoot/src/core",'-include',"$pedBuild/default.h")
foreach($pedSanUnit in @('AudioPed','AudioGeometry')) {
 & clang -std=c11 @pedSanArgs -c "$pedRoot/src/audio/$pedSanUnit.c" $(if($pedSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestPedRealSubmit'}) -o "$pedSanBuild/$pedSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $pedSanUnit"}
}
& clang -std=c11 @pedSanArgs -c "$pedBuild/caller.c" -o "$pedSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @pedSanArgs -c "$pedBuild/fast_hash.c" -o "$pedSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$pedSanCommon=@("$pedSanBuild/fast_hash.o","$pedSanBuild/AudioGeometry.o")
$pedSanResults=@()
foreach($pedSanVariant in @('before','after')) {
 $pedSanExtra=if($pedSanVariant -eq 'after') {@("$pedSanBuild/AudioPed.o","$pedSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @pedSanArgs "$pedBuild/$pedSanVariant.cpp" @pedSanCommon @pedSanExtra -o "$pedSanBuild/$pedSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $pedSanResults += & "$pedSanBuild/$pedSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $pedSanVariant"}
}
if($pedSanResults.Count -ne 2 -or $pedSanResults[0] -ne $pedSanResults[1]) {throw 'Sanitized comparison failed'}
$pedSanResults | ConvertTo-Json | Set-Content "$pedBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($pedSanResults[1])"
