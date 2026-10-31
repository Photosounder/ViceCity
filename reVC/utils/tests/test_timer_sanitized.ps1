$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$timerRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$timerBuild=Join-Path $timerRoot 'build/timer-c-tests'

$timerSources=$timerBuild
$timerBuild=Join-Path $timerBuild 'sanitized'
New-Item -ItemType Directory -Force $timerBuild | Out-Null
$timerRuns=@()
foreach($timerCompiler in @('clang')) {
 foreach($timerOptimization in @('O1')) {
  $timerCpp=if($timerCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($timerShape in @('default','vanilla','portable','portable-vanilla')) {
   $timerArgs=@('-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$timerRoot/src/audio","-I$timerRoot/src/core",'-include',"$timerSources/$timerShape.h")
   foreach($timerUnit in @('Timer')) {
    & $timerCompiler -std=c11 -pedantic-errors "-$timerOptimization" @timerArgs -c "$timerRoot/src/core/$timerUnit.c" -o "$timerBuild/$timerUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $timerUnit"}
   }
   & $timerCompiler -std=c11 -pedantic-errors "-$timerOptimization" @timerArgs -c "$timerSources/caller.c" -o "$timerBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $timerCommon=@()
   & $timerCpp -std=c++17 "-$timerOptimization" @timerArgs "$timerSources/before.cpp" @timerCommon -o "$timerBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $timerCpp -std=c++17 "-$timerOptimization" @timerArgs "$timerSources/after.cpp" @timerCommon "$timerBuild/Timer.o" "$timerBuild/caller.o" -o "$timerBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $timerBefore=& "$timerBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $timerAfter=& "$timerBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $timerBefore -ne $timerAfter) {throw "Timer comparison failed: $timerBefore / $timerAfter"}
   $timerRuns += "$timerCompiler-$timerOptimization-$timerShape : $timerAfter"
   Write-Output $timerRuns[-1]
  }
 }
}
$timerRuns | ConvertTo-Json | Set-Content "$timerBuild/runtime-runs.json"
