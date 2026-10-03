$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$streamRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$streamBuild=Join-Path $streamRoot 'build/audio-stream-native-c-tests'
New-Item -ItemType Directory -Force $streamBuild | Out-Null
$streamFixture=Join-Path $streamRoot 'src/audio/oal/stream.c'
$streamArgs=@('-std=c11','-fsyntax-only','-pedantic-errors','-Wno-multichar','-Wno-strict-prototypes','-DAUDIO_OAL',"-I$streamRoot/src/core",'-IC:/msys/home/github/CITAlloc',"-I$streamRoot/src/audio/oal","-I$streamRoot/vendor/openal-soft/include","-I$streamRoot/vendor/ogg/include","-I$streamRoot/vendor/opus/include","-I$streamRoot/vendor/opusfile/include","-I$streamRoot/vendor/libsndfile/include","-I$streamRoot/vendor/mpg123/include")
$streamRuns=@()
foreach($streamCompiler in @('clang','gcc')) {
 foreach($streamThread in @('single','native','posix')) {
  foreach($streamDecoders in @('WAV','all')) {
   foreach($streamTarget in @('x86_64','i686')) {
    if($streamTarget -eq 'i686' -and ($streamCompiler -ne 'clang' -or $streamThread -eq 'single' -or $streamDecoders -eq 'WAV')) {continue}
    $streamConfig=Join-Path $streamBuild "config-$streamThread-$streamDecoders.h"
    $streamConfigText="#include `"$($streamRoot.Replace('\','/'))/src/core/config.h`"`n#undef MULTITHREADED_AUDIO`n#undef AUDIO_OAL_USE_MPG123`n#undef AUDIO_OAL_USE_SNDFILE`n#undef AUDIO_OAL_USE_OPUS`n"
    if($streamThread -ne 'single') {$streamConfigText += "#define MULTITHREADED_AUDIO`n"}
    if($streamDecoders -eq 'all') {$streamConfigText += "#define AUDIO_OAL_USE_SNDFILE`n#define AUDIO_OAL_USE_MPG123`n#define AUDIO_OAL_USE_OPUS`n"}
    $streamConfigText | Set-Content $streamConfig
    $streamExtra=@('-include',$streamConfig)
    if($streamThread -eq 'posix') {$streamExtra += '-DAUDIO_THREAD_USE_POSIX'}
    if($streamTarget -eq 'i686') {$streamExtra += @('-target','i686-w64-windows-gnu')}
    $streamName="$streamCompiler-$streamThread-$streamDecoders-$streamTarget"
    $streamLog=& $streamCompiler @streamArgs @streamExtra $streamFixture 2>&1
    $streamLog | Set-Content (Join-Path $streamBuild "$streamName.log")
    if($LASTEXITCODE -ne 0) {$streamLog; throw "C syntax check failed: $streamName"}
    $streamRuns += $streamName
   }
  }
 }
}
$streamRuns | ConvertTo-Json | Set-Content (Join-Path $streamBuild 'c-syntax-runs.json')
Write-Output "Full production stream C11 syntax checks passed: $($streamRuns.Count)"

$streamRuntime=Join-Path $streamBuild 'stream_runtime.c'
& C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe (Join-Path $PSScriptRoot 'audio_stream_c_fixture.py') $streamRuntime --runtime
if($LASTEXITCODE -ne 0) {throw 'Failed to prepare the production C lifecycle fixture'}
$streamRuntimeArgs=@($streamArgs | Where-Object {$_ -ne '-fsyntax-only'})
$streamRuntimeRuns=@()
foreach($streamCompiler in @('clang','gcc')) {
 foreach($streamThread in @('single','native','posix')) {
  foreach($streamOptimization in @('O0','O2')) {
   $streamConfig=Join-Path $streamBuild "config-$streamThread-WAV.h"
   $streamExtra=@('-include',$streamConfig)
   if($streamThread -eq 'posix') {$streamExtra += '-DAUDIO_THREAD_USE_POSIX'}
   $streamName="$streamCompiler-$streamThread-$streamOptimization"
   $streamExe=Join-Path $streamBuild "$streamName.exe"
   & $streamCompiler @streamRuntimeArgs @streamExtra "-$streamOptimization" -pthread -ffunction-sections -fdata-sections '-Wl,--gc-sections' $streamRuntime -o $streamExe
   if($LASTEXITCODE -ne 0) {throw "C lifecycle compilation failed: $streamName"}
   & $streamExe
   if($LASTEXITCODE -ne 0) {throw "C lifecycle runtime failed: $streamName"}
   $streamRuntimeRuns += $streamName
  }
 }
}
$streamRuntimeRuns | ConvertTo-Json | Set-Content (Join-Path $streamBuild 'c-runtime-runs.json')

# Link a separately compiled C stream with a C++ caller of the public header
foreach($streamBackend in @('native','posix')) {
 $streamLinkArgs=@('-include',(Join-Path $streamBuild "config-$streamBackend-WAV.h"),"-I$streamRoot/src/audio/oal","-I$streamRoot/src/core","-I$streamRoot/vendor/openal-soft/include","-I$streamRoot/vendor/mpg123/include",'-IC:/msys/home/github/CITAlloc','-DAUDIO_OAL','-pthread')
 if($streamBackend -eq 'posix') {$streamLinkArgs += '-DAUDIO_THREAD_USE_POSIX'}
 $streamObject=Join-Path $streamBuild "linkage-$streamBackend.o"
 $streamExe=Join-Path $streamBuild "linkage-$streamBackend.exe"
 & clang -std=c11 -O2 -Wno-multichar -Dmain=AudioStream_CTest @streamLinkArgs -c $streamRuntime -o $streamObject
 if($LASTEXITCODE -ne 0) {throw 'C linkage object compilation failed'}
 & clang++ -std=c++17 @streamLinkArgs (Join-Path $PSScriptRoot 'audio_stream_linkage.cpp') $streamObject -o $streamExe
 if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
 & $streamExe
 if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ runtime failed'}
}
