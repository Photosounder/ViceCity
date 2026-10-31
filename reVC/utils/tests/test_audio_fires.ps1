$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$firesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$firesBuild=Join-Path $firesRoot 'build/audio-fires-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_fires_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Fire source audit failed'}
$firesRuns=@()
foreach($firesCompiler in @('clang','gcc')) {
 foreach($firesOptimization in @('O0','O2')) {
  $firesCpp=if($firesCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($firesShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale','no-reflections')) {
   $firesArgs=@("-I$firesRoot/src/audio","-I$firesRoot/src/core",'-include',"$firesBuild/$firesShape.h")
   foreach($firesUnit in @('AudioFires','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
    & $firesCompiler -std=c11 -pedantic-errors "-$firesOptimization" @firesArgs -c "$firesRoot/src/audio/$firesUnit.c" $(if($firesUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestFiresRealSubmit'}) -o "$firesBuild/$firesUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $firesUnit"}
   }
   & $firesCompiler -std=c11 -pedantic-errors "-$firesOptimization" @firesArgs -c "$firesBuild/caller.c" -o "$firesBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $firesCompiler -std=c11 -O2 -c "$firesBuild/fast_hash.c" -o "$firesBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $firesCommon=@("$firesBuild/fast_hash.o","$firesBuild/AudioMath.o","$firesBuild/AudioGeometry.o","$firesBuild/AudioRequests.o","$firesBuild/AudioSoundQueue.o")
   & $firesCpp -std=c++17 "-$firesOptimization" @firesArgs "$firesBuild/before.cpp" @firesCommon -o "$firesBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $firesCpp -std=c++17 "-$firesOptimization" @firesArgs "$firesBuild/after.cpp" @firesCommon "$firesBuild/AudioFires.o" "$firesBuild/caller.o" -o "$firesBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $firesBefore=& "$firesBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $firesAfter=& "$firesBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $firesBefore -ne $firesAfter) {throw "Fire comparison failed: $firesBefore / $firesAfter"}
   $firesRuns += "$firesCompiler-$firesOptimization-$firesShape : $firesAfter"
   Write-Output $firesRuns[-1]
  }
 }
}
$firesRuns | ConvertTo-Json | Set-Content "$firesBuild/runtime-runs.json"
$firesSanBuild=Join-Path $firesBuild 'sanitized'
New-Item -ItemType Directory -Path $firesSanBuild -Force | Out-Null
$firesSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$firesRoot/src/audio","-I$firesRoot/src/core",'-include',"$firesBuild/default.h")
foreach($firesSanUnit in @('AudioFires','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @firesSanArgs -c "$firesRoot/src/audio/$firesSanUnit.c" $(if($firesSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestFiresRealSubmit'}) -o "$firesSanBuild/$firesSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $firesSanUnit"}
}
& clang -std=c11 @firesSanArgs -c "$firesBuild/caller.c" -o "$firesSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @firesSanArgs -c "$firesBuild/fast_hash.c" -o "$firesSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$firesSanCommon=@("$firesSanBuild/fast_hash.o","$firesSanBuild/AudioMath.o","$firesSanBuild/AudioGeometry.o","$firesSanBuild/AudioRequests.o","$firesSanBuild/AudioSoundQueue.o")
$firesSanResults=@()
foreach($firesSanVariant in @('before','after')) {
 $firesSanExtra=if($firesSanVariant -eq 'after') {@("$firesSanBuild/AudioFires.o","$firesSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @firesSanArgs "$firesBuild/$firesSanVariant.cpp" @firesSanCommon @firesSanExtra -o "$firesSanBuild/$firesSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $firesSanResults += & "$firesSanBuild/$firesSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $firesSanVariant"}
}
if($firesSanResults.Count -ne 2 -or $firesSanResults[0] -ne $firesSanResults[1]) {throw 'Sanitized comparison failed'}
$firesSanResults | ConvertTo-Json | Set-Content "$firesBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($firesSanResults[1])"
