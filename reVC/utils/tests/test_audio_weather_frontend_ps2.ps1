$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$weatherFrontendRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$weatherFrontendBuild=Join-Path $weatherFrontendRoot 'build/audio-weather-frontend-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_weather_frontend_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'WeatherFrontend source audit failed'}
$weatherFrontendSanBuild=Join-Path $weatherFrontendBuild 'sanitized-ps2'
New-Item -ItemType Directory -Path $weatherFrontendSanBuild -Force | Out-Null
$weatherFrontendSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$weatherFrontendRoot/src/audio","-I$weatherFrontendRoot/src/core",'-include',"$weatherFrontendBuild/ps2.h")
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
$weatherFrontendSanResults | ConvertTo-Json | Set-Content "$weatherFrontendBuild/sanitizer-ps2-runs.json"
Write-Output "Sanitized comparison passed: $($weatherFrontendSanResults[1])"
