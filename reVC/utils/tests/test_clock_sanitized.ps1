$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$clockRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$clockBuild=Join-Path $clockRoot 'build/clock-c-tests'

$clockSources=$clockBuild
$clockBuild=Join-Path $clockBuild 'sanitized'
New-Item -ItemType Directory -Force $clockBuild | Out-Null
$clockRuns=@()
foreach($clockCompiler in @('clang')) {
 foreach($clockOptimization in @('O1')) {
  $clockCpp=if($clockCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($clockShape in @('default','master')) {
   $clockArgs=@('-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$clockRoot/src/audio","-I$clockRoot/src/core",'-include',"$clockSources/$clockShape.h")
   foreach($clockUnit in @('Clock')) {
    & $clockCompiler -std=c11 -pedantic-errors "-$clockOptimization" @clockArgs -c "$clockRoot/src/core/$clockUnit.c" -o "$clockBuild/$clockUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $clockUnit"}
   }
   & $clockCompiler -std=c11 -pedantic-errors "-$clockOptimization" @clockArgs -c "$clockSources/caller.c" -o "$clockBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $clockCommon=@()
   & $clockCpp -std=c++17 "-$clockOptimization" @clockArgs "$clockSources/before.cpp" @clockCommon -o "$clockBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $clockCpp -std=c++17 "-$clockOptimization" @clockArgs "$clockSources/after.cpp" @clockCommon "$clockBuild/Clock.o" "$clockBuild/caller.o" -o "$clockBuild/after.exe"
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
