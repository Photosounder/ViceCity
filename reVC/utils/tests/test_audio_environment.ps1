$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$environmentRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$environmentBuild=Join-Path $environmentRoot 'build/audio-environment-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_environment_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Environment source audit failed'}
$environmentRuns=@()
foreach($environmentCompiler in @('clang','gcc')) {
 foreach($environmentOptimization in @('O0','O2')) {
  $environmentCpp=if($environmentCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($environmentShape in @('default','vanilla','no-reverb','no-external','ps2')) {
   $environmentArgs=@("-I$environmentRoot/src/audio","-I$environmentRoot/src/core",'-include',"$environmentBuild/$environmentShape.h")
   foreach($environmentUnit in @('AudioEnvironment')) {
    & $environmentCompiler -std=c11 -pedantic-errors "-$environmentOptimization" @environmentArgs -c "$environmentRoot/src/audio/$environmentUnit.c" -o "$environmentBuild/$environmentUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $environmentUnit"}
   }
   & $environmentCompiler -std=c11 -pedantic-errors "-$environmentOptimization" @environmentArgs -c "$environmentBuild/caller.c" -o "$environmentBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $environmentCommon=@()
   & $environmentCpp -std=c++17 "-$environmentOptimization" @environmentArgs "$environmentBuild/before.cpp" @environmentCommon -o "$environmentBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $environmentCpp -std=c++17 "-$environmentOptimization" @environmentArgs "$environmentBuild/after.cpp" @environmentCommon "$environmentBuild/AudioEnvironment.o" "$environmentBuild/caller.o" -o "$environmentBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $environmentBefore=& "$environmentBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $environmentAfter=& "$environmentBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $environmentBefore -ne $environmentAfter) {throw "Environment comparison failed: $environmentBefore / $environmentAfter"}
   $environmentRuns += "$environmentCompiler-$environmentOptimization-$environmentShape : $environmentAfter"
   Write-Output $environmentRuns[-1]
  }
 }
}
$environmentRuns | ConvertTo-Json | Set-Content "$environmentBuild/runtime-runs.json"
$environmentSanBuild=Join-Path $environmentBuild 'sanitized'
New-Item -ItemType Directory -Path $environmentSanBuild -Force | Out-Null
$environmentSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$environmentRoot/src/audio","-I$environmentRoot/src/core",'-include',"$environmentBuild/default.h")
foreach($environmentSanUnit in @('AudioEnvironment')) {
 & clang -std=c11 @environmentSanArgs -c "$environmentRoot/src/audio/$environmentSanUnit.c" -o "$environmentSanBuild/$environmentSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $environmentSanUnit"}
}
& clang -std=c11 @environmentSanArgs -c "$environmentBuild/caller.c" -o "$environmentSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$environmentSanCommon=@()
$environmentSanResults=@()
foreach($environmentSanVariant in @('before','after')) {
 $environmentSanExtra=if($environmentSanVariant -eq 'after') {@("$environmentSanBuild/AudioEnvironment.o","$environmentSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @environmentSanArgs "$environmentBuild/$environmentSanVariant.cpp" @environmentSanCommon @environmentSanExtra -o "$environmentSanBuild/$environmentSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $environmentSanResults += & "$environmentSanBuild/$environmentSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $environmentSanVariant"}
}
if($environmentSanResults.Count -ne 2 -or $environmentSanResults[0] -ne $environmentSanResults[1]) {throw 'Sanitized comparison failed'}
$environmentSanResults | ConvertTo-Json | Set-Content "$environmentBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($environmentSanResults[1])"
