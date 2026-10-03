$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$deviceRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$deviceBuild=Join-Path $deviceRoot 'build/audio-device-c-tests'
New-Item -ItemType Directory -Force $deviceBuild | Out-Null
$devicePython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $devicePython (Join-Path $PSScriptRoot 'audio_device_fixture.py') (Join-Path $deviceBuild 'current.c')
if($LASTEXITCODE -ne 0) {throw 'C device fixture failed'}
& $devicePython (Join-Path $PSScriptRoot 'audio_device_fixture.py') (Join-Path $deviceBuild 'baseline.cpp') --baseline
if($LASTEXITCODE -ne 0) {throw 'Saved C++ effect fixture failed'}
$deviceIncludes=@("-I$deviceRoot","-I$deviceRoot/src/audio/oal","-I$deviceRoot/src/core","-I$deviceRoot/src/audio","-I$deviceRoot/src/audio/eax","-I$deviceRoot/vendor/openal-soft/include")
$deviceRuns=@()
foreach($deviceCompiler in @('clang','gcc')) {
 foreach($deviceOptimization in @('O0','O2')) {
  $deviceCppCompiler=if($deviceCompiler -eq 'clang') {'clang++'} else {'g++'}
  $deviceBaselineExe=Join-Path $deviceBuild "$deviceCompiler-baseline-$deviceOptimization.exe"
  & $deviceCppCompiler -std=c++17 "-$deviceOptimization" @deviceIncludes (Join-Path $deviceBuild 'baseline.cpp') -o $deviceBaselineExe
  if($LASTEXITCODE -ne 0) {throw 'Original C++ devices compilation failed'}
  $deviceBaseline=& $deviceBaselineExe
  if($LASTEXITCODE -ne 0) {throw 'Original C++ devices runtime failed'}
  foreach($deviceLanguage in @('c','c++')) {
   $deviceName="$deviceCompiler-$($deviceLanguage.Replace('+','p'))-$deviceOptimization"
   $deviceExe=Join-Path $deviceBuild "$deviceName.exe"
   $deviceStandard=if($deviceLanguage -eq 'c') {'-std=c11'} else {'-std=c++17'}
   $deviceLanguageArgs=@()
   if($deviceLanguage -eq 'c') {$deviceLanguageArgs += '-Wno-strict-prototypes'}
   & $deviceCompiler -x $deviceLanguage $deviceStandard "-$deviceOptimization" -Wall -Wextra -Wno-unused-variable @deviceLanguageArgs -IC:/msys/home/github/CITAlloc @deviceIncludes (Join-Path $deviceBuild 'current.c') -o $deviceExe
   if($LASTEXITCODE -ne 0) {throw "C device compilation failed: $deviceName"}
   $deviceOutput=& $deviceExe
   if($LASTEXITCODE -ne 0) {throw "C device runtime failed: $deviceName"}
   if(Compare-Object @($deviceBaseline) @($deviceOutput)) {throw "Effects differ from original C++ trace: $deviceName"}
   $deviceRuns += $deviceName
   Write-Output "Device behavior and exact baseline trace passed: $deviceName"
  }
 }
}
$deviceRuns | ConvertTo-Json | Set-Content (Join-Path $deviceBuild 'device-runtime-runs.json')


# Link the actual C device-list implementation with a C++ caller of its public header
$deviceLinkArgs=@('-DAUDIO_OAL','-IC:/msys/home/github/CITAlloc')+$deviceIncludes
$deviceObject=Join-Path $deviceBuild 'linkage.o'
$deviceExe=Join-Path $deviceBuild 'linkage.exe'
& clang -std=c11 -O2 -Dmain=AudioDevice_CTest @deviceLinkArgs -c (Join-Path $deviceBuild 'current.c') -o $deviceObject
if($LASTEXITCODE -ne 0) {throw 'Device C linkage object failed'}
& clang++ -std=c++17 @deviceLinkArgs (Join-Path $PSScriptRoot 'audio_device_linkage.cpp') $deviceObject -o $deviceExe
if($LASTEXITCODE -ne 0) {throw 'Device C/C++ link failed'}
$deviceOutput=& $deviceExe
if($LASTEXITCODE -ne 0) {throw 'Device C/C++ runtime failed'}
Write-Output 'Device C/C++ linkage and runtime passed'
