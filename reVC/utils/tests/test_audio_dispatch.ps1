$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$dispatchRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$dispatchBuild=Join-Path $dispatchRoot 'build/audio-dispatch-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_dispatch_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Dispatch source audit failed'}
$dispatchRuns=@()
foreach($dispatchCompiler in @('clang','gcc')) {
 foreach($dispatchOptimization in @('O0','O2')) {
  $dispatchCpp=if($dispatchCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($dispatchShape in @('default','no-reverb','no-external','bridge','ps2')) {
   $dispatchArgs=@("-I$dispatchRoot/src/audio","-I$dispatchRoot/src/core",'-include',"$dispatchBuild/$dispatchShape.h")
   foreach($dispatchUnit in @('AudioDispatch')) {
    & $dispatchCompiler -std=c11 -pedantic-errors "-$dispatchOptimization" @dispatchArgs -c "$dispatchRoot/src/audio/$dispatchUnit.c" -o "$dispatchBuild/$dispatchUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $dispatchUnit"}
   }
   & $dispatchCompiler -std=c11 -pedantic-errors "-$dispatchOptimization" @dispatchArgs -c "$dispatchBuild/caller.c" -o "$dispatchBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $dispatchCommon=@()
   & $dispatchCpp -std=c++17 "-$dispatchOptimization" @dispatchArgs "$dispatchBuild/before.cpp" @dispatchCommon -o "$dispatchBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $dispatchCpp -std=c++17 "-$dispatchOptimization" @dispatchArgs "$dispatchBuild/after.cpp" @dispatchCommon "$dispatchBuild/AudioDispatch.o" "$dispatchBuild/caller.o" -o "$dispatchBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $dispatchBefore=& "$dispatchBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $dispatchAfter=& "$dispatchBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $dispatchBefore -ne $dispatchAfter) {throw "Dispatch comparison failed: $dispatchBefore / $dispatchAfter"}
   $dispatchRuns += "$dispatchCompiler-$dispatchOptimization-$dispatchShape : $dispatchAfter"
   Write-Output $dispatchRuns[-1]
  }
 }
}
$dispatchRuns | ConvertTo-Json | Set-Content "$dispatchBuild/runtime-runs.json"
$dispatchSanBuild=Join-Path $dispatchBuild 'sanitized'
New-Item -ItemType Directory -Path $dispatchSanBuild -Force | Out-Null
$dispatchSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$dispatchRoot/src/audio","-I$dispatchRoot/src/core",'-include',"$dispatchBuild/default.h")
foreach($dispatchSanUnit in @('AudioDispatch')) {
 & clang -std=c11 @dispatchSanArgs -c "$dispatchRoot/src/audio/$dispatchSanUnit.c" -o "$dispatchSanBuild/$dispatchSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $dispatchSanUnit"}
}
& clang -std=c11 @dispatchSanArgs -c "$dispatchBuild/caller.c" -o "$dispatchSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$dispatchSanCommon=@()
$dispatchSanResults=@()
foreach($dispatchSanVariant in @('before','after')) {
 $dispatchSanExtra=if($dispatchSanVariant -eq 'after') {@("$dispatchSanBuild/AudioDispatch.o","$dispatchSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @dispatchSanArgs "$dispatchBuild/$dispatchSanVariant.cpp" @dispatchSanCommon @dispatchSanExtra -o "$dispatchSanBuild/$dispatchSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $dispatchSanResults += & "$dispatchSanBuild/$dispatchSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $dispatchSanVariant"}
}
if($dispatchSanResults.Count -ne 2 -or $dispatchSanResults[0] -ne $dispatchSanResults[1]) {throw 'Sanitized comparison failed'}
$dispatchSanResults | ConvertTo-Json | Set-Content "$dispatchBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($dispatchSanResults[1])"
