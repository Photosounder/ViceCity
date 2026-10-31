$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$weatherFrontendRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$weatherFrontendBuild=Join-Path $weatherFrontendRoot 'build/audio-weather-frontend-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_weather_frontend_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'WeatherFrontend source audit failed'}
$weatherFrontendRuns=@()
foreach($weatherFrontendCompiler in @('clang','gcc')) {
 foreach($weatherFrontendOptimization in @('O0','O2')) {
  $weatherFrontendCpp=if($weatherFrontendCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($weatherFrontendShape in @('default','vanilla','no-reverb','no-external','ps2','no-timescale','no-reflections')) {
   $weatherFrontendArgs=@("-I$weatherFrontendRoot/src/audio","-I$weatherFrontendRoot/src/core",'-include',"$weatherFrontendBuild/$weatherFrontendShape.h")
   foreach($weatherFrontendUnit in @('AudioWeatherFrontend','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
    & $weatherFrontendCompiler -std=c11 -pedantic-errors "-$weatherFrontendOptimization" @weatherFrontendArgs -c "$weatherFrontendRoot/src/audio/$weatherFrontendUnit.c" $(if($weatherFrontendUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestWeatherFrontendRealSubmit'}) -o "$weatherFrontendBuild/$weatherFrontendUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $weatherFrontendUnit"}
   }
   & $weatherFrontendCompiler -std=c11 -pedantic-errors "-$weatherFrontendOptimization" @weatherFrontendArgs -c "$weatherFrontendBuild/caller.c" -o "$weatherFrontendBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $weatherFrontendCompiler -std=c11 -O2 -c "$weatherFrontendBuild/fast_hash.c" -o "$weatherFrontendBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $weatherFrontendCommon=@("$weatherFrontendBuild/fast_hash.o","$weatherFrontendBuild/AudioMath.o","$weatherFrontendBuild/AudioGeometry.o","$weatherFrontendBuild/AudioRequests.o","$weatherFrontendBuild/AudioSoundQueue.o")
   & $weatherFrontendCpp -std=c++17 "-$weatherFrontendOptimization" @weatherFrontendArgs "$weatherFrontendBuild/before.cpp" @weatherFrontendCommon -o "$weatherFrontendBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $weatherFrontendCpp -std=c++17 "-$weatherFrontendOptimization" @weatherFrontendArgs "$weatherFrontendBuild/after.cpp" @weatherFrontendCommon "$weatherFrontendBuild/AudioWeatherFrontend.o" "$weatherFrontendBuild/caller.o" -o "$weatherFrontendBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $weatherFrontendBefore=& "$weatherFrontendBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $weatherFrontendAfter=& "$weatherFrontendBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $weatherFrontendBefore -ne $weatherFrontendAfter) {throw "WeatherFrontend comparison failed: $weatherFrontendBefore / $weatherFrontendAfter"}
   $weatherFrontendRuns += "$weatherFrontendCompiler-$weatherFrontendOptimization-$weatherFrontendShape : $weatherFrontendAfter"
   Write-Output $weatherFrontendRuns[-1]
  }
 }
}
$weatherFrontendRuns | ConvertTo-Json | Set-Content "$weatherFrontendBuild/runtime-runs.json"
$weatherFrontendSanBuild=Join-Path $weatherFrontendBuild 'sanitized'
New-Item -ItemType Directory -Path $weatherFrontendSanBuild -Force | Out-Null
$weatherFrontendSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$weatherFrontendRoot/src/audio","-I$weatherFrontendRoot/src/core",'-include',"$weatherFrontendBuild/default.h")
foreach($weatherFrontendSanUnit in @('AudioWeatherFrontend','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @weatherFrontendSanArgs -c "$weatherFrontendRoot/src/audio/$weatherFrontendSanUnit.c" $(if($weatherFrontendSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestWeatherFrontendRealSubmit'}) -o "$weatherFrontendSanBuild/$weatherFrontendSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $weatherFrontendSanUnit"}
}
& clang -std=c11 @weatherFrontendSanArgs -c "$weatherFrontendBuild/caller.c" -o "$weatherFrontendSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @weatherFrontendSanArgs -c "$weatherFrontendBuild/fast_hash.c" -o "$weatherFrontendSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$weatherFrontendSanCommon=@("$weatherFrontendSanBuild/fast_hash.o","$weatherFrontendSanBuild/AudioMath.o","$weatherFrontendSanBuild/AudioGeometry.o","$weatherFrontendSanBuild/AudioRequests.o","$weatherFrontendSanBuild/AudioSoundQueue.o")
$weatherFrontendSanResults=@()
foreach($weatherFrontendSanVariant in @('before','after')) {
 $weatherFrontendSanExtra=if($weatherFrontendSanVariant -eq 'after') {@("$weatherFrontendSanBuild/AudioWeatherFrontend.o","$weatherFrontendSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @weatherFrontendSanArgs "$weatherFrontendBuild/$weatherFrontendSanVariant.cpp" @weatherFrontendSanCommon @weatherFrontendSanExtra -o "$weatherFrontendSanBuild/$weatherFrontendSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $weatherFrontendSanResults += & "$weatherFrontendSanBuild/$weatherFrontendSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $weatherFrontendSanVariant"}
}
if($weatherFrontendSanResults.Count -ne 2 -or $weatherFrontendSanResults[0] -ne $weatherFrontendSanResults[1]) {throw 'Sanitized comparison failed'}
$weatherFrontendSanResults | ConvertTo-Json | Set-Content "$weatherFrontendBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($weatherFrontendSanResults[1])"
