param([ValidateSet("OAL","MSS")][string]$HostBackend="OAL")
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$hostRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$hostBuild=Join-Path $hostRoot 'build/audio-sample-native-c-tests'
New-Item -ItemType Directory -Force $hostBuild | Out-Null
$hostPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $hostPython (Join-Path $PSScriptRoot 'audio_sample_host_fixture.py') "$hostBuild/host-fixture.cpp"
if($LASTEXITCODE -ne 0) {throw 'Host fixture generation failed'}
$hostArgs=@("-DAUDIO_$HostBackend","-I$hostRoot/src/core","-I$hostRoot/src/audio","-I$hostRoot/src/audio/oal","-I$hostRoot/src/audio/eax","-I$hostRoot/vendor/openal-soft/include")
$hostRuns=@()
foreach($hostCompiler in @('clang','gcc')) {
 foreach($hostOptimization in @('O0','O2')) {
  $hostCpp=if($hostCompiler -eq 'clang') {'clang++'} else {'g++'}
  & $hostCompiler -std=c11 "-$hostOptimization" @hostArgs -c (Join-Path $PSScriptRoot 'audio_sample_host.c') -o "$hostBuild/host-caller.o"
  if($LASTEXITCODE -ne 0) {throw 'Host C caller compile failed'}
  & $hostCompiler -std=c11 "-$hostOptimization" @hostArgs -c "$hostRoot/src/audio/eax/eax-util.c" -o "$hostBuild/eax.o"
  if($LASTEXITCODE -ne 0) {throw 'Production EAX C compilation failed'}
  & $hostCpp -std=c++17 "-$hostOptimization" @hostArgs "$hostBuild/host-fixture.cpp" "$hostBuild/eax.o" "$hostBuild/host-caller.o" -o "$hostBuild/host-linkage.exe"
  if($LASTEXITCODE -ne 0) {throw 'Host C/C++ and EAX link failed'}
  & "$hostBuild/host-linkage.exe"
  if($LASTEXITCODE -ne 0) {throw 'Host C/C++ runtime failed'}
  $hostRuns += "$HostBackend-$hostCompiler-$hostOptimization"
  Write-Output "Game-state callbacks and production EAX linkage passed: $($hostRuns[-1])"
 }
}
$hostReportName=if($HostBackend -eq 'OAL') {'host-runtime-runs.json'} else {'host-runtime-runs-MSS.json'}
$hostRuns | ConvertTo-Json | Set-Content "$hostBuild/$hostReportName"
