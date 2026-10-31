$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$cullZonesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$cullZonesBuild=Join-Path $cullZonesRoot 'build/cull-zones-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/cull_zones_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'CullZones source audit failed'}
$cullZonesRuns=@()
foreach($cullZonesCompiler in @('clang','gcc')) {
 foreach($cullZonesOptimization in @('O0','O2')) {
  $cullZonesCpp=if($cullZonesCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($cullZonesShape in @('default','master')) {
   $cullZonesArgs=@("-I$cullZonesBuild","-I$cullZonesRoot/src/audio","-I$cullZonesRoot/src/core",'-include',"$cullZonesBuild/$cullZonesShape.h")
   foreach($cullZonesUnit in @('ZoneCull')) {
    & $cullZonesCompiler -std=c11 -pedantic-errors "-$cullZonesOptimization" @cullZonesArgs -c "$cullZonesRoot/src/core/$cullZonesUnit.c" -o "$cullZonesBuild/$cullZonesUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $cullZonesUnit"}
   }
   & $cullZonesCompiler -std=c11 -pedantic-errors "-$cullZonesOptimization" @cullZonesArgs -c "$cullZonesBuild/caller.c" -o "$cullZonesBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $cullZonesCommon=@()
   & $cullZonesCpp -std=c++17 "-$cullZonesOptimization" @cullZonesArgs "$cullZonesBuild/before.cpp" @cullZonesCommon -o "$cullZonesBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $cullZonesCpp -std=c++17 "-$cullZonesOptimization" @cullZonesArgs "$cullZonesBuild/after.cpp" @cullZonesCommon "$cullZonesBuild/ZoneCull.o" "$cullZonesBuild/caller.o" -o "$cullZonesBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $cullZonesBefore=& "$cullZonesBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $cullZonesAfter=& "$cullZonesBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $cullZonesBefore -ne $cullZonesAfter) {throw "CullZones comparison failed: $cullZonesBefore / $cullZonesAfter"}
   $cullZonesRuns += "$cullZonesCompiler-$cullZonesOptimization-$cullZonesShape : $cullZonesAfter"
   Write-Output $cullZonesRuns[-1]
  }
 }
}
$cullZonesRuns | ConvertTo-Json | Set-Content "$cullZonesBuild/runtime-runs.json"
