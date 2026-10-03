$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$channelRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$channelBuild=Join-Path $channelRoot 'build/audio-channel-c-tests'
New-Item -ItemType Directory -Force $channelBuild | Out-Null
$channelPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $channelPython (Join-Path $PSScriptRoot 'audio_channel_fixture.py') (Join-Path $channelBuild 'current.c')
if($LASTEXITCODE -ne 0) {throw 'C channel fixture failed'}
& $channelPython (Join-Path $PSScriptRoot 'audio_channel_fixture.py') (Join-Path $channelBuild 'baseline.cpp') --baseline
if($LASTEXITCODE -ne 0) {throw 'Saved C++ effect fixture failed'}
$channelIncludes=@("-I$channelRoot","-I$channelRoot/src/audio/oal","-I$channelRoot/src/core","-I$channelRoot/src/audio","-I$channelRoot/src/audio/eax","-I$channelRoot/vendor/openal-soft/include")
$channelRuns=@()
foreach($channelCompiler in @('clang','gcc')) {
 foreach($channelOptimization in @('O0','O2')) {
  $channelCppCompiler=if($channelCompiler -eq 'clang') {'clang++'} else {'g++'}
  $channelBaselineExe=Join-Path $channelBuild "$channelCompiler-baseline-$channelOptimization.exe"
  & $channelCppCompiler -std=c++17 "-$channelOptimization" @channelIncludes (Join-Path $channelBuild 'baseline.cpp') -o $channelBaselineExe
  if($LASTEXITCODE -ne 0) {throw 'Original C++ channels compilation failed'}
  $channelBaseline=& $channelBaselineExe
  if($LASTEXITCODE -ne 0) {throw 'Original C++ channels runtime failed'}
  foreach($channelLanguage in @('c','c++')) {
   $channelName="$channelCompiler-$($channelLanguage.Replace('+','p'))-$channelOptimization"
   $channelExe=Join-Path $channelBuild "$channelName.exe"
   $channelStandard=if($channelLanguage -eq 'c') {'-std=c11'} else {'-std=c++17'}
   & $channelCompiler -x $channelLanguage $channelStandard "-$channelOptimization" -Wall -Wextra @channelIncludes (Join-Path $channelBuild 'current.c') -o $channelExe
   if($LASTEXITCODE -ne 0) {throw "C channel compilation failed: $channelName"}
   $channelOutput=& $channelExe
   if($LASTEXITCODE -ne 0) {throw "C channel runtime failed: $channelName"}
   if(Compare-Object @($channelBaseline) @($channelOutput)) {throw "Effects differ from original C++ trace: $channelName"}
   $channelRuns += $channelName
   Write-Output "Channel behavior and exact baseline trace passed: $channelName"
  }
 }
}
$channelRuns | ConvertTo-Json | Set-Content (Join-Path $channelBuild 'channel-runtime-runs.json')

# Verify public C linkage with a separately compiled C object and a C++ caller
$channelLinkArgs=@('-DAUDIO_OAL')+$channelIncludes
$channelObject=Join-Path $channelBuild 'linkage.o'
$channelExe=Join-Path $channelBuild 'linkage.exe'
& clang -std=c11 -O2 -Dmain=AudioChannel_CTest @channelLinkArgs -c (Join-Path $channelBuild 'current.c') -o $channelObject
if($LASTEXITCODE -ne 0) {throw 'Channel C linkage object failed'}
& clang++ -std=c++17 @channelLinkArgs (Join-Path $PSScriptRoot 'audio_channel_linkage.cpp') $channelObject -o $channelExe
if($LASTEXITCODE -ne 0) {throw 'Channel C/C++ link failed'}
$channelOutput=& $channelExe
if($LASTEXITCODE -ne 0) {throw 'Channel C/C++ runtime failed'}
Write-Output 'Channel C/C++ linkage and runtime passed'
