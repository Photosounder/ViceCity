$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$audioRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$audioBuild=Join-Path $audioRoot 'build/audio-eax-null-c-tests'
[IO.File]::WriteAllText("$audioBuild/null-compat.h",[IO.File]::ReadAllText((Join-Path $PSScriptRoot 'audio_null_compat.h')))
$audioIncludes=@("-I$audioRoot/src/core","-I$audioRoot/src/audio","-I$audioRoot/src/audio/eax","-I$audioRoot/src/audio/oal","-I$audioRoot/vendor/openal-soft/include")
$audioGameIncludes=@("-I$audioRoot/src","-I$audioRoot/vendor/librw",'-IC:/msys/home','-IC:/msys/home/github/CITAlloc','-ID:/VC_libs/include')
$audioGameIncludes += @(Get-ChildItem "$audioRoot/src" -Directory -Recurse | ForEach-Object {'-I'+$_.FullName})
$audioRuns=@()
foreach($audioCompiler in @('clang','gcc')) {
 foreach($audioOptimization in @('O0','O2')) {
  $audioCpp=if($audioCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($audioModule in @('eax','null')) {
   $audioFile=if($audioModule -eq 'eax') {'eax/eax-util'} else {'sampman_null'}
   $audioFixture=if($audioModule -eq 'eax') {'audio_eax_math.c'} else {'audio_null_backend.c'}
   $audioFlags=@()
   if($audioModule -eq 'eax') {$audioFlags += '-DAUDIO_OAL'} else {$audioFlags += @('-DLIBRW','-DRW_D3D9','-DUSE_D3D9')}
   $audioBaselineSource="$audioBuild/before/src/audio/$audioFile.cpp"
   if($audioModule -eq 'null') {
    # Keep the saved backend bodies intact while supplying only the game-header definitions they actually use
    $audioBaselineSource="$audioBuild/null-baseline.cpp"
    $audioSaved=[IO.File]::ReadAllText("$audioBuild/before/src/audio/sampman_null.cpp").Replace('#include "common.h"','#include "null-compat.h"').Replace('#include "AudioManager.h"','#include "AudioReflectionTypes.h"')
    [IO.File]::WriteAllText($audioBaselineSource,$audioSaved.Replace("`r`n","`n"))
   }
   & $audioCpp -std=c++17 "-$audioOptimization" @audioIncludes @audioGameIncludes @audioFlags -c $audioBaselineSource -o "$audioBuild/baseline.o"
   if($LASTEXITCODE -ne 0) {throw 'Saved C++ module compile failed'}
   & $audioCpp -std=c++17 "-$audioOptimization" @audioIncludes @audioFlags -x c++ (Join-Path $PSScriptRoot $audioFixture) -x none "$audioBuild/baseline.o" -o "$audioBuild/baseline.exe"
   if($LASTEXITCODE -ne 0) {throw 'Saved C++ module link failed'}
   $audioBaseline=& "$audioBuild/baseline.exe"
   if($LASTEXITCODE -ne 0) {throw 'Saved C++ module runtime failed'}
   & $audioCompiler -std=c11 "-$audioOptimization" @audioIncludes @audioFlags -c "$audioRoot/src/audio/$audioFile.c" -o "$audioBuild/current.o"
   if($LASTEXITCODE -ne 0) {throw 'Production C module compile failed'}
   foreach($audioLanguage in @('c','c++')) {
    $audioStandard=if($audioLanguage -eq 'c') {'-std=c11'} else {'-std=c++17'}
    $audioCaller=if($audioLanguage -eq 'c') {$audioCompiler} else {$audioCpp}
    & $audioCaller $audioStandard "-$audioOptimization" @audioIncludes @audioFlags -x $audioLanguage (Join-Path $PSScriptRoot $audioFixture) -x none "$audioBuild/current.o" -o "$audioBuild/current.exe"
    if($LASTEXITCODE -ne 0) {throw 'C module caller link failed'}
    $audioOutput=& "$audioBuild/current.exe"
    if($LASTEXITCODE -ne 0) {throw 'C module caller runtime failed'}
    if(Compare-Object @($audioBaseline) @($audioOutput)) {throw 'Production C behavior differs from saved C++ module'}
    $audioRuns += "$audioModule-$audioCompiler-$audioLanguage-$audioOptimization"
    Write-Output "Production module and baseline parity passed: $($audioRuns[-1])"
   }
  }
 }
}
$audioRuns | ConvertTo-Json | Set-Content "$audioBuild/runtime-runs.json"
