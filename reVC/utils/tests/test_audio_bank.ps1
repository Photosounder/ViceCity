$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$bankRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$bankBuild=Join-Path $bankRoot 'build/audio-bank-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_bank_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Bank source audit failed'}
$bankRuns=@()
foreach($bankCompiler in @('clang','gcc')) {
 foreach($bankOptimization in @('O0','O2')) {
  $bankCpp=if($bankCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($bankShape in @('ps2','ps2-no-reverb','ps2-no-reflections','desktop')) {
   $bankArgs=@("-I$bankRoot/src/audio","-I$bankRoot/src/core",'-include',"$bankBuild/$bankShape.h")
   foreach($bankUnit in @('AudioBank')) {
    & $bankCompiler -std=c11 -pedantic-errors "-$bankOptimization" @bankArgs -c "$bankRoot/src/audio/$bankUnit.c" -o "$bankBuild/$bankUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $bankUnit"}
   }
   & $bankCompiler -std=c11 -pedantic-errors "-$bankOptimization" @bankArgs -c "$bankBuild/caller.c" -o "$bankBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   & $bankCompiler -std=c11 -O2 -c "$bankBuild/fast_hash.c" -o "$bankBuild/fast_hash.o"
   if($LASTEXITCODE -ne 0) {throw 'Observer compile failed'}
   $bankCommon=@("$bankBuild/fast_hash.o")
   & $bankCpp -std=c++17 "-$bankOptimization" @bankArgs "$bankBuild/before.cpp" @bankCommon -o "$bankBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $bankCpp -std=c++17 "-$bankOptimization" @bankArgs "$bankBuild/after.cpp" @bankCommon "$bankBuild/AudioBank.o" "$bankBuild/caller.o" -o "$bankBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $bankBefore=& "$bankBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $bankAfter=& "$bankBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $bankBefore -ne $bankAfter) {throw "Bank comparison failed: $bankBefore / $bankAfter"}
   $bankRuns += "$bankCompiler-$bankOptimization-$bankShape : $bankAfter"
   Write-Output $bankRuns[-1]
  }
 }
}
$bankRuns | ConvertTo-Json | Set-Content "$bankBuild/runtime-runs.json"
$bankSanBuild=Join-Path $bankBuild 'sanitized'
New-Item -ItemType Directory -Path $bankSanBuild -Force | Out-Null
$bankSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$bankRoot/src/audio","-I$bankRoot/src/core",'-include',"$bankBuild/ps2.h")
foreach($bankSanUnit in @('AudioBank')) {
 & clang -std=c11 @bankSanArgs -c "$bankRoot/src/audio/$bankSanUnit.c" -o "$bankSanBuild/$bankSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $bankSanUnit"}
}
& clang -std=c11 @bankSanArgs -c "$bankBuild/caller.c" -o "$bankSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @bankSanArgs -c "$bankBuild/fast_hash.c" -o "$bankSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$bankSanCommon=@("$bankSanBuild/fast_hash.o")
$bankSanResults=@()
foreach($bankSanVariant in @('before','after')) {
 $bankSanExtra=if($bankSanVariant -eq 'after') {@("$bankSanBuild/AudioBank.o","$bankSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @bankSanArgs "$bankBuild/$bankSanVariant.cpp" @bankSanCommon @bankSanExtra -o "$bankSanBuild/$bankSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $bankSanResults += & "$bankSanBuild/$bankSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $bankSanVariant"}
}
if($bankSanResults.Count -ne 2 -or $bankSanResults[0] -ne $bankSanResults[1]) {throw 'Sanitized comparison failed'}
$bankSanResults | ConvertTo-Json | Set-Content "$bankBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($bankSanResults[1])"
