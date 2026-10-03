$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$crimeRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$crimeBuild=Join-Path $crimeRoot 'build/audio-crime-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_crime_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Crime fixture generation failed'}
$crimeArgs=@("-I$crimeRoot/src/audio", "-I$crimeRoot/src/core")
$crimeRuns=@()
foreach($crimeCompiler in @('clang','gcc')) {
 foreach($crimeOptimization in @('O0','O2')) {
  $crimeCpp=if($crimeCompiler -eq 'clang') {'clang++'} else {'g++'}
  & $crimeCpp -std=c++17 "-$crimeOptimization" @crimeArgs "$crimeBuild/before.cpp" -o "$crimeBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline crime compilation failed'}
  & $crimeCompiler -std=c11 -pedantic-errors "-$crimeOptimization" @crimeArgs -c "$crimeRoot/src/audio/AudioCrimes.c" -o "$crimeBuild/crimes.o"
  if($LASTEXITCODE -ne 0) {throw 'Production crime C compilation failed'}
  & $crimeCompiler -std=c11 -pedantic-errors "-$crimeOptimization" @crimeArgs -c "$crimeBuild/caller.c" -o "$crimeBuild/caller.o"
  if($LASTEXITCODE -ne 0) {throw 'Crime C caller compilation failed'}
  & $crimeCpp -std=c++17 "-$crimeOptimization" @crimeArgs "$crimeBuild/after.cpp" "$crimeBuild/crimes.o" "$crimeBuild/caller.o" -o "$crimeBuild/after.exe"
  if($LASTEXITCODE -ne 0) {throw 'Mixed crime C/C++ link failed'}
  $crimeExpected=& "$crimeBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline crime runtime failed'}
  $crimeActual=& "$crimeBuild/after.exe"
  if($LASTEXITCODE -ne 0 -or $crimeExpected -ne $crimeActual) {throw "Crime trace mismatch: $crimeExpected / $crimeActual"}
  $crimeRuns += "$crimeCompiler-$crimeOptimization : $crimeActual"
  Write-Output "Record layouts, actual report adapters, cooldowns, and aging matched: $($crimeRuns[-1])"
 }
}
$crimeRuns | ConvertTo-Json | Set-Content "$crimeBuild/runtime-runs.json"
