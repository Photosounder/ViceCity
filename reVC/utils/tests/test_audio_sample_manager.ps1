$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$managerRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$managerBuild=Join-Path $managerRoot 'build/audio-sample-manager-c-tests'
New-Item -ItemType Directory -Force $managerBuild | Out-Null
$managerPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $managerPython (Join-Path $PSScriptRoot 'audio_sample_manager_fixture.py') "$managerBuild/current.c"
if($LASTEXITCODE -ne 0) {throw 'Current fixture generation failed'}
& $managerPython (Join-Path $PSScriptRoot 'audio_sample_manager_fixture.py') "$managerBuild/baseline.cpp" --baseline
if($LASTEXITCODE -ne 0) {throw 'Baseline fixture generation failed'}
$managerIncludes=@("-I$managerRoot","-I$managerRoot/src/core","-I$managerRoot/src/audio",'-DAUDIO_OAL')
$managerRuns=@()
foreach($managerCompiler in @('clang','gcc')) {
 foreach($managerOptimization in @('O0','O2')) {
  $managerCpp=if($managerCompiler -eq 'clang') {'clang++'} else {'g++'}
  & $managerCpp -std=c++17 "-$managerOptimization" @managerIncludes "$managerBuild/baseline.cpp" -o "$managerBuild/baseline.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline compilation failed'}
  $managerBaseline=& "$managerBuild/baseline.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline execution failed'}
  foreach($managerLanguage in @('c','c++')) {
   $managerStandard=if($managerLanguage -eq 'c') {'-std=c11'} else {'-std=c++17'}
   & $managerCompiler -x $managerLanguage $managerStandard "-$managerOptimization" @managerIncludes "$managerBuild/current.c" -o "$managerBuild/current.exe"
   if($LASTEXITCODE -ne 0) {throw 'Current compilation failed'}
   $managerOutput=& "$managerBuild/current.exe"
   if($LASTEXITCODE -ne 0) {throw 'Current execution failed'}
   if(Compare-Object @($managerBaseline) @($managerOutput)) {throw 'Manager trace differs from saved C++ class'}
   $managerRuns += "$managerCompiler-$managerLanguage-$managerOptimization"
   Write-Output "Manager state, metadata and channel baseline passed: $($managerRuns[-1])"
  }
 }
}
$managerRuns | ConvertTo-Json | Set-Content "$managerBuild/manager-runtime-runs.json"
