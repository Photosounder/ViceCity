$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$dmaRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$dmaBuild=Join-Path $dmaRoot 'build/audio-dma-c-api-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_dma_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'DMAudio fixture generation failed'}
$dmaArgs=@("-I$dmaRoot/src/audio", "-I$dmaRoot/src/core")
$dmaRuns=@()
foreach($dmaCompiler in @('clang','gcc')) {
 foreach($dmaOptimization in @('O0','O2')) {
  $dmaCpp=if($dmaCompiler -eq 'clang') {'clang++'} else {'g++'}
  & $dmaCompiler -std=c11 -pedantic-errors "-$dmaOptimization" @dmaArgs -c "$dmaBuild/caller.c" -o "$dmaBuild/caller.o"
  if($LASTEXITCODE -ne 0) {throw 'DMAudio C caller compilation failed'}
  & $dmaCpp -std=c++17 "-$dmaOptimization" @dmaArgs "$dmaBuild/before.cpp" -o "$dmaBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline facade compilation failed'}
  & $dmaCpp -std=c++17 "-$dmaOptimization" @dmaArgs "$dmaBuild/after.cpp" "$dmaBuild/caller.o" -o "$dmaBuild/after.exe"
  if($LASTEXITCODE -ne 0) {throw 'C facade linkage failed'}
  $dmaExpected=& "$dmaBuild/before.exe"
  $dmaActual=& "$dmaBuild/after.exe"
  if($dmaActual -ne $dmaExpected -or $LASTEXITCODE -ne 0) {throw "Facade trace mismatch: $dmaExpected / $dmaActual"}
  $dmaRuns += "$dmaCompiler-$dmaOptimization : $dmaActual"
  Write-Output "All 62 facade exports and boundary traces matched: $($dmaRuns[-1])"
 }
}
$dmaRuns | ConvertTo-Json | Set-Content "$dmaBuild/runtime-runs.json"
