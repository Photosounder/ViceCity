$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$clockRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$clockBuild=Join-Path $clockRoot 'build/clock-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/clock_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Clock source audit failed'}
$clockRuns=@()
foreach($clockCompiler in @('clang','gcc')) {
 foreach($clockOptimization in @('O0','O2')) {
  $clockCpp=if($clockCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($clockShape in @('default','vanilla','master','ps2')) {
   $clockArgs=@("-I$clockRoot/src/audio","-I$clockRoot/src/core",'-include',"$clockBuild/$clockShape.h")
   foreach($clockUnit in @('Clock')) {
    & $clockCompiler -std=c11 -pedantic-errors "-$clockOptimization" @clockArgs -c "$clockRoot/src/core/$clockUnit.c" -o "$clockBuild/$clockUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $clockUnit"}
   }
   & $clockCompiler -std=c11 -pedantic-errors "-$clockOptimization" @clockArgs -c "$clockBuild/caller.c" -o "$clockBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $clockCommon=@()
   & $clockCpp -std=c++17 "-$clockOptimization" @clockArgs "$clockBuild/before.cpp" @clockCommon -o "$clockBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $clockCpp -std=c++17 "-$clockOptimization" @clockArgs "$clockBuild/after.cpp" @clockCommon "$clockBuild/Clock.o" "$clockBuild/caller.o" -o "$clockBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $clockBefore=& "$clockBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $clockAfter=& "$clockBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $clockBefore -ne $clockAfter) {throw "Clock comparison failed: $clockBefore / $clockAfter"}
   $clockRuns += "$clockCompiler-$clockOptimization-$clockShape : $clockAfter"
   Write-Output $clockRuns[-1]
  }
 }
}
$clockRuns | ConvertTo-Json | Set-Content "$clockBuild/runtime-runs.json"
