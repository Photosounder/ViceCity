$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$zonesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$zonesBuild=Join-Path $zonesRoot 'build/zones-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/zones_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Zones source audit failed'}
$zonesRuns=@()
foreach($zonesCompiler in @('clang','gcc')) {
 foreach($zonesOptimization in @('O0','O2')) {
  $zonesCpp=if($zonesCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($zonesShape in @('default','vanilla','master','squeeze')) {
   $zonesArgs=@("-I$zonesBuild","-I$zonesRoot/src/save","-I$zonesRoot/src/core",'-include',"$zonesBuild/$zonesShape.h")
   foreach($zonesUnit in @('Zones')) {
    & $zonesCompiler -std=c11 -pedantic-errors "-$zonesOptimization" @zonesArgs -c "$zonesRoot/src/core/$zonesUnit.c" -o "$zonesBuild/$zonesUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $zonesUnit"}
   }
   & $zonesCompiler -std=c11 -pedantic-errors "-$zonesOptimization" @zonesArgs -c "$zonesBuild/caller.c" -o "$zonesBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $zonesCommon=@()
   & $zonesCpp -std=c++17 "-$zonesOptimization" @zonesArgs "$zonesBuild/before.cpp" @zonesCommon -o "$zonesBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $zonesCpp -std=c++17 "-$zonesOptimization" @zonesArgs "$zonesBuild/after.cpp" @zonesCommon "$zonesBuild/Zones.o" "$zonesBuild/caller.o" -o "$zonesBuild/after.exe"
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
