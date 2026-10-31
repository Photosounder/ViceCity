$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$zonesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$zonesBuild=Join-Path $zonesRoot 'build/zones-c-tests'

$zonesSources=$zonesBuild
$zonesBuild=Join-Path $zonesBuild 'sanitized'
New-Item -ItemType Directory -Force $zonesBuild | Out-Null
$zonesRuns=@()
foreach($zonesCompiler in @('clang')) {
 foreach($zonesOptimization in @('O1')) {
  $zonesCpp=if($zonesCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($zonesShape in @('default','vanilla','master','squeeze')) {
   $zonesArgs=@("-I$zonesSources",'-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$zonesRoot/src/save","-I$zonesRoot/src/core",'-include',"$zonesSources/$zonesShape.h")
   foreach($zonesUnit in @('Zones')) {
    & $zonesCompiler -std=c11 -pedantic-errors "-$zonesOptimization" @zonesArgs -c "$zonesRoot/src/core/$zonesUnit.c" -o "$zonesBuild/$zonesUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $zonesUnit"}
   }
   & $zonesCompiler -std=c11 -pedantic-errors "-$zonesOptimization" @zonesArgs -c "$zonesSources/caller.c" -o "$zonesBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $zonesCommon=@()
   & $zonesCpp -std=c++17 "-$zonesOptimization" @zonesArgs "$zonesSources/before.cpp" @zonesCommon -o "$zonesBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $zonesCpp -std=c++17 "-$zonesOptimization" @zonesArgs "$zonesSources/after.cpp" @zonesCommon "$zonesBuild/Zones.o" "$zonesBuild/caller.o" -o "$zonesBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $zonesBefore=& "$zonesBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $zonesAfter=& "$zonesBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $zonesBefore -ne $zonesAfter) {throw "Zones comparison failed: $zonesBefore / $zonesAfter"}
   $zonesRuns += "$zonesCompiler-$zonesOptimization-$zonesShape : $zonesAfter"
   Write-Output $zonesRuns[-1]
  }
 }
}
$zonesRuns | ConvertTo-Json | Set-Content "$zonesBuild/runtime-runs.json"
