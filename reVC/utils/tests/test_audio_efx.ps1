$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$efxRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$efxBuild=Join-Path $efxRoot 'build/audio-efx-c-tests'
New-Item -ItemType Directory -Force $efxBuild | Out-Null
$efxPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $efxPython (Join-Path $PSScriptRoot 'audio_efx_fixture.py') (Join-Path $efxBuild 'current.c')
if($LASTEXITCODE -ne 0) {throw 'C effect fixture failed'}
& $efxPython (Join-Path $PSScriptRoot 'audio_efx_fixture.py') (Join-Path $efxBuild 'baseline.cpp') --baseline
if($LASTEXITCODE -ne 0) {throw 'Saved C++ effect fixture failed'}
$efxIncludes=@("-I$efxRoot","-I$efxRoot/src/audio/oal","-I$efxRoot/src/audio/eax","-I$efxRoot/vendor/openal-soft/include")
$efxRuns=@()
foreach($efxCompiler in @('clang','gcc')) {
 foreach($efxOptimization in @('O0','O2')) {
  $efxCppCompiler=if($efxCompiler -eq 'clang') {'clang++'} else {'g++'}
  $efxBaselineExe=Join-Path $efxBuild "$efxCompiler-baseline-$efxOptimization.exe"
  & $efxCppCompiler -std=c++17 "-$efxOptimization" @efxIncludes (Join-Path $efxBuild 'baseline.cpp') -o $efxBaselineExe
  if($LASTEXITCODE -ne 0) {throw 'Original C++ effects compilation failed'}
  $efxBaseline=& $efxBaselineExe
  if($LASTEXITCODE -ne 0) {throw 'Original C++ effects runtime failed'}
  foreach($efxLanguage in @('c','c++')) {
   $efxName="$efxCompiler-$($efxLanguage.Replace('+','p'))-$efxOptimization"
   $efxExe=Join-Path $efxBuild "$efxName.exe"
   $efxStandard=if($efxLanguage -eq 'c') {'-std=c11'} else {'-std=c++17'}
   & $efxCompiler -x $efxLanguage $efxStandard "-$efxOptimization" -Wall -Wextra -Werror @efxIncludes (Join-Path $efxBuild 'current.c') -o $efxExe
   if($LASTEXITCODE -ne 0) {throw "C effect compilation failed: $efxName"}
   $efxOutput=& $efxExe
   if($LASTEXITCODE -ne 0) {throw "C effect runtime failed: $efxName"}
   if(Compare-Object @($efxBaseline) @($efxOutput)) {throw "Effects differ from original C++ trace: $efxName"}
   $efxRuns += $efxName
   Write-Output "Effect procedure loading and exact baseline trace passed: $efxName"
  }
 }
}
$efxRuns | ConvertTo-Json | Set-Content (Join-Path $efxBuild 'effect-runtime-runs.json')
