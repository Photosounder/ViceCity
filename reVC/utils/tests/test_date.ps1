param([switch]$Sanitized)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$dateRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$dateBuild=Join-Path $dateRoot 'build/date-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/date_fixture.py"
if($LASTEXITCODE -ne 0){throw 'Date source audit failed'}
$dateRuns=@()
$dateCompilers=if($Sanitized){@('clang')}else{@('clang','gcc')}
$dateOptimizations=if($Sanitized){@('O1')}else{@('O0','O2')}
foreach($dateCompiler in $dateCompilers) {
 foreach($dateOptimization in $dateOptimizations) {
  $dateCpp=if($dateCompiler -eq 'clang') {'clang++'} else {'g++'}
  $dateArgs=@("-I$dateBuild","-I$dateRoot/src/save")
  if($Sanitized){$dateArgs+=@('-fsanitize=undefined','-fsanitize-trap=all')}
  & $dateCompiler -std=c11 -pedantic-errors "-$dateOptimization" @dateArgs -c "$dateRoot/src/save/Date.c" -o "$dateBuild/Date.o"
  if($LASTEXITCODE -ne 0){throw 'C date compile failed'}
  & $dateCompiler -std=c11 -pedantic-errors "-$dateOptimization" @dateArgs -c "$dateBuild/caller.c" -o "$dateBuild/caller.o"
  if($LASTEXITCODE -ne 0){throw 'C date caller failed'}
  & $dateCpp -std=c++17 "-$dateOptimization" @dateArgs "$dateBuild/before.cpp" -o "$dateBuild/before.exe"
  if($LASTEXITCODE -ne 0){throw 'Original date compile failed'}
  & $dateCpp -std=c++17 "-$dateOptimization" @dateArgs "$dateBuild/after.cpp" "$dateBuild/Date.o" "$dateBuild/caller.o" -o "$dateBuild/after.exe"
  if($LASTEXITCODE -ne 0){throw 'Mixed date link failed'}
  $dateBefore=& "$dateBuild/before.exe"
  if($LASTEXITCODE -ne 0){throw 'Original date runtime failed'}
  $dateAfter=& "$dateBuild/after.exe"
  if($LASTEXITCODE -ne 0 -or $dateBefore -ne $dateAfter){throw "Date comparison failed: $dateBefore / $dateAfter"}
  $dateRuns += "$dateCompiler-$dateOptimization : $dateAfter"
  Write-Output $dateRuns[-1]
 }
}
$dateResultsFile=if($Sanitized){'sanitized-runs.json'}else{'runtime-runs.json'}
$dateRuns | ConvertTo-Json | Set-Content "$dateBuild/$dateResultsFile"
