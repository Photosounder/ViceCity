$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$bankRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$bankBuild=Join-Path $bankRoot 'build/audio-bank-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_bank_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Bank source audit failed'}
$bankSanBuild=Join-Path $bankBuild 'sanitized-desktop'
New-Item -ItemType Directory -Path $bankSanBuild -Force | Out-Null
$bankSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$bankRoot/src/audio","-I$bankRoot/src/core",'-include',"$bankBuild/desktop.h")
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
$bankSanResults | ConvertTo-Json | Set-Content "$bankBuild/sanitizer-desktop-runs.json"
Write-Output "Sanitized comparison passed: $($bankSanResults[1])"
