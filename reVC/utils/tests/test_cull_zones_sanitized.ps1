$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$cullZonesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$cullZonesBuild=Join-Path $cullZonesRoot 'build/cull-zones-c-tests'

$cullZonesSources=$cullZonesBuild
$cullZonesBuild=Join-Path $cullZonesBuild 'sanitized'
New-Item -ItemType Directory -Force $cullZonesBuild | Out-Null
$cullZonesRuns=@()
foreach($cullZonesCompiler in @('clang')) {
 foreach($cullZonesOptimization in @('O1')) {
  $cullZonesCpp=if($cullZonesCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($cullZonesShape in @('default','master')) {
   $cullZonesArgs=@("-I$cullZonesSources",'-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$cullZonesRoot/src/audio","-I$cullZonesRoot/src/core",'-include',"$cullZonesSources/$cullZonesShape.h")
   foreach($cullZonesUnit in @('ZoneCull')) {
    & $cullZonesCompiler -std=c11 -pedantic-errors "-$cullZonesOptimization" @cullZonesArgs -c "$cullZonesRoot/src/core/$cullZonesUnit.c" -o "$cullZonesBuild/$cullZonesUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $cullZonesUnit"}
   }
   & $cullZonesCompiler -std=c11 -pedantic-errors "-$cullZonesOptimization" @cullZonesArgs -c "$cullZonesSources/caller.c" -o "$cullZonesBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $cullZonesCommon=@()
   & $cullZonesCpp -std=c++17 "-$cullZonesOptimization" @cullZonesArgs "$cullZonesSources/before.cpp" @cullZonesCommon -o "$cullZonesBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $cullZonesCpp -std=c++17 "-$cullZonesOptimization" @cullZonesArgs "$cullZonesSources/after.cpp" @cullZonesCommon "$cullZonesBuild/ZoneCull.o" "$cullZonesBuild/caller.o" -o "$cullZonesBuild/after.exe"
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
