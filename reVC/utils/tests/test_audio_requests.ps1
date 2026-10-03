$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$requestsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$requestsBuild=Join-Path $requestsRoot 'build/audio-requests-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_requests_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Requests source audit failed'}
$requestsRuns=@()
foreach($requestsCompiler in @('clang','gcc')) {
 foreach($requestsOptimization in @('O0','O2')) {
  $requestsCpp=if($requestsCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($requestsShape in @('default','vanilla','unscaled','no-external','no-reflections','ps2')) {
   $requestsArgs=@("-I$requestsRoot/src/audio","-I$requestsRoot/src/core",'-include',"$requestsBuild/$requestsShape.h")
   foreach($requestsUnit in @('AudioMath','AudioSoundQueue','AudioRequests')) {
    & $requestsCompiler -std=c11 -pedantic-errors "-$requestsOptimization" @requestsArgs -c "$requestsRoot/src/audio/$requestsUnit.c" -o "$requestsBuild/$requestsUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $requestsUnit"}
   }
   & $requestsCompiler -std=c11 -pedantic-errors "-$requestsOptimization" @requestsArgs -c "$requestsBuild/caller.c" -o "$requestsBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $requestsCpp -std=c++17 "-$requestsOptimization" @requestsArgs "$requestsBuild/before.cpp" "$requestsBuild/AudioMath.o" "$requestsBuild/AudioSoundQueue.o" -o "$requestsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $requestsCpp -std=c++17 "-$requestsOptimization" @requestsArgs "$requestsBuild/after.cpp" "$requestsBuild/AudioMath.o" "$requestsBuild/AudioSoundQueue.o" "$requestsBuild/AudioRequests.o" "$requestsBuild/caller.o" -o "$requestsBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $requestsBefore=& "$requestsBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $requestsAfter=& "$requestsBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $requestsBefore -ne $requestsAfter) {throw "Requests comparison failed: $requestsBefore / $requestsAfter"}
   $requestsRuns += "$requestsCompiler-$requestsOptimization-$requestsShape : $requestsAfter"
   Write-Output $requestsRuns[-1]
  }
 }
}
$requestsRuns | ConvertTo-Json | Set-Content "$requestsBuild/runtime-runs.json"
