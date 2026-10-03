$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$activeRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$activeBuild=Join-Path $activeRoot 'build/audio-active-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_active_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Active source audit failed'}
$activeRuns=@()
foreach($activeCompiler in @('clang','gcc')) {
 foreach($activeOptimization in @('O0','O2')) {
  $activeCpp=if($activeCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($activeShape in @('default','vanilla','unscaled','no-external','no-reflections','ps2')) {
   $activeArgs=@("-I$activeRoot/src/audio","-I$activeRoot/src/core",'-include',"$activeBuild/$activeShape.h")
   foreach($activeUnit in @('AudioActive','AudioGeometry','AudioMath')) {
    & $activeCompiler -std=c11 -pedantic-errors "-$activeOptimization" @activeArgs -c "$activeRoot/src/audio/$activeUnit.c" -o "$activeBuild/$activeUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $activeUnit"}
   }
   & $activeCompiler -std=c11 -pedantic-errors "-$activeOptimization" @activeArgs -c "$activeBuild/caller.c" -o "$activeBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $activeCommon=@("$activeBuild/AudioGeometry.o","$activeBuild/AudioMath.o")
   & $activeCpp -std=c++17 "-$activeOptimization" @activeArgs "$activeBuild/before.cpp" @activeCommon -o "$activeBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $activeCpp -std=c++17 "-$activeOptimization" @activeArgs "$activeBuild/after.cpp" @activeCommon "$activeBuild/AudioActive.o" "$activeBuild/caller.o" -o "$activeBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $activeBefore=& "$activeBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $activeAfter=& "$activeBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $activeBefore -ne $activeAfter) {throw "Active comparison failed: $activeBefore / $activeAfter"}
   $activeRuns += "$activeCompiler-$activeOptimization-$activeShape : $activeAfter"
   Write-Output $activeRuns[-1]
  }
 }
}
$activeRuns | ConvertTo-Json | Set-Content "$activeBuild/runtime-runs.json"
$activeSanBuild=Join-Path $activeBuild 'sanitized'
New-Item -ItemType Directory -Path $activeSanBuild -Force | Out-Null
$activeSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$activeRoot/src/audio","-I$activeRoot/src/core",'-include',"$activeBuild/default.h")
foreach($activeSanUnit in @('AudioActive','AudioGeometry','AudioMath')) {
 & clang -std=c11 @activeSanArgs -c "$activeRoot/src/audio/$activeSanUnit.c" -o "$activeSanBuild/$activeSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $activeSanUnit"}
}
& clang -std=c11 @activeSanArgs -c "$activeBuild/caller.c" -o "$activeSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$activeSanCommon=@("$activeSanBuild/AudioGeometry.o","$activeSanBuild/AudioMath.o")
$activeSanResults=@()
foreach($activeSanVariant in @('before','after')) {
 $activeSanExtra=if($activeSanVariant -eq 'after') {@("$activeSanBuild/AudioActive.o","$activeSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @activeSanArgs "$activeBuild/$activeSanVariant.cpp" @activeSanCommon @activeSanExtra -o "$activeSanBuild/$activeSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $activeSanResults += & "$activeSanBuild/$activeSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $activeSanVariant"}
}
if($activeSanResults.Count -ne 2 -or $activeSanResults[0] -ne $activeSanResults[1]) {throw 'Sanitized comparison failed'}
$activeSanResults | ConvertTo-Json | Set-Content "$activeBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($activeSanResults[1])"
