$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$collisionReportRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$collisionReportBuild=Join-Path $collisionReportRoot 'build/audio-collision-report-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_collision_report_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'CollisionReport source audit failed'}
$collisionReportRuns=@()
foreach($collisionReportCompiler in @('clang','gcc')) {
 foreach($collisionReportOptimization in @('O0','O2')) {
  $collisionReportCpp=if($collisionReportCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($collisionReportShape in @('default','vanilla','no-reverb','no-external','ps2')) {
   $collisionReportArgs=@("-I$collisionReportRoot/src/audio","-I$collisionReportRoot/src/core",'-include',"$collisionReportBuild/$collisionReportShape.h")
   foreach($collisionReportUnit in @('AudioCollisionReport','AudioCollisionQueue','AudioGeometry')) {
    & $collisionReportCompiler -std=c11 -pedantic-errors "-$collisionReportOptimization" @collisionReportArgs -c "$collisionReportRoot/src/audio/$collisionReportUnit.c" -o "$collisionReportBuild/$collisionReportUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $collisionReportUnit"}
   }
   & $collisionReportCompiler -std=c11 -pedantic-errors "-$collisionReportOptimization" @collisionReportArgs -c "$collisionReportBuild/caller.c" -o "$collisionReportBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $collisionReportCommon=@("$collisionReportBuild/AudioCollisionQueue.o","$collisionReportBuild/AudioGeometry.o")
   & $collisionReportCpp -std=c++17 "-$collisionReportOptimization" @collisionReportArgs "$collisionReportBuild/before.cpp" @collisionReportCommon -o "$collisionReportBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $collisionReportCpp -std=c++17 "-$collisionReportOptimization" @collisionReportArgs "$collisionReportBuild/after.cpp" @collisionReportCommon "$collisionReportBuild/AudioCollisionReport.o" "$collisionReportBuild/caller.o" -o "$collisionReportBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $collisionReportBefore=& "$collisionReportBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $collisionReportAfter=& "$collisionReportBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $collisionReportBefore -ne $collisionReportAfter) {throw "CollisionReport comparison failed: $collisionReportBefore / $collisionReportAfter"}
   $collisionReportRuns += "$collisionReportCompiler-$collisionReportOptimization-$collisionReportShape : $collisionReportAfter"
   Write-Output $collisionReportRuns[-1]
  }
 }
}
$collisionReportRuns | ConvertTo-Json | Set-Content "$collisionReportBuild/runtime-runs.json"
$collisionReportSanBuild=Join-Path $collisionReportBuild 'sanitized'
New-Item -ItemType Directory -Path $collisionReportSanBuild -Force | Out-Null
$collisionReportSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$collisionReportRoot/src/audio","-I$collisionReportRoot/src/core",'-include',"$collisionReportBuild/default.h")
foreach($collisionReportSanUnit in @('AudioCollisionReport','AudioCollisionQueue','AudioGeometry')) {
 & clang -std=c11 @collisionReportSanArgs -c "$collisionReportRoot/src/audio/$collisionReportSanUnit.c" -o "$collisionReportSanBuild/$collisionReportSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $collisionReportSanUnit"}
}
& clang -std=c11 @collisionReportSanArgs -c "$collisionReportBuild/caller.c" -o "$collisionReportSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$collisionReportSanCommon=@("$collisionReportSanBuild/AudioCollisionQueue.o","$collisionReportSanBuild/AudioGeometry.o")
$collisionReportSanResults=@()
foreach($collisionReportSanVariant in @('before','after')) {
 $collisionReportSanExtra=if($collisionReportSanVariant -eq 'after') {@("$collisionReportSanBuild/AudioCollisionReport.o","$collisionReportSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @collisionReportSanArgs "$collisionReportBuild/$collisionReportSanVariant.cpp" @collisionReportSanCommon @collisionReportSanExtra -o "$collisionReportSanBuild/$collisionReportSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $collisionReportSanResults += & "$collisionReportSanBuild/$collisionReportSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $collisionReportSanVariant"}
}
if($collisionReportSanResults.Count -ne 2 -or $collisionReportSanResults[0] -ne $collisionReportSanResults[1]) {throw 'Sanitized comparison failed'}
$collisionReportSanResults | ConvertTo-Json | Set-Content "$collisionReportBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($collisionReportSanResults[1])"
