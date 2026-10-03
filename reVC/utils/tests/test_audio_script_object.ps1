$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$scriptRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$scriptBuild=Join-Path $scriptRoot 'build/audio-script-object-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_script_object_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Script object fixture generation failed'}
$scriptArgs=@("-I$scriptRoot/src/audio", "-I$scriptRoot/src/core", "-I$scriptRoot/src/save", '-include', "$scriptRoot/src/core/config.h")
$scriptRuns=@()
foreach($scriptCompiler in @('clang','gcc')) {
 foreach($scriptOptimization in @('O0','O2')) {
  $scriptCpp=if($scriptCompiler -eq 'clang') {'clang++'} else {'g++'}
  & $scriptCpp -std=c++17 "-$scriptOptimization" @scriptArgs -c "$scriptBuild/counter.cpp" -o "$scriptBuild/counter.o"
  if($LASTEXITCODE -ne 0) {throw 'Game C++ save counter compilation failed'}
  & $scriptCpp -std=c++17 "-$scriptOptimization" @scriptArgs -c "$scriptBuild/cpp-caller.cpp" -o "$scriptBuild/cpp-caller.o"
  if($LASTEXITCODE -ne 0) {throw 'C++ caller compilation failed'}
  & $scriptCompiler -std=c11 -pedantic-errors "-$scriptOptimization" @scriptArgs "$scriptRoot/src/audio/AudioScriptObject.c" "$scriptBuild/after.c" "$scriptBuild/counter.o" "$scriptBuild/cpp-caller.o" -o "$scriptBuild/after.exe"
  if($LASTEXITCODE -ne 0) {throw 'Production C script object compilation failed'}
  & $scriptCpp -std=c++17 "-$scriptOptimization" @scriptArgs "$scriptBuild/before.cpp" -o "$scriptBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline script object compilation failed'}
  $scriptExpected=& "$scriptBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline script object runtime failed'}
  $scriptActual=& "$scriptBuild/after.exe"
  if($LASTEXITCODE -ne 0 -or $scriptExpected -ne $scriptActual) {throw "Script object trace mismatch: $scriptExpected / $scriptActual"}
  $scriptRuns += "$scriptCompiler-$scriptOptimization : $scriptActual"
  Write-Output "Save bytes, generations, resets, and one-shot traces matched: $($scriptRuns[-1])"
 }
}
$scriptRuns | ConvertTo-Json | Set-Content "$scriptBuild/runtime-runs.json"
