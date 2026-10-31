param([switch]$Sanitized)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$curvesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$curvesBuild=Join-Path $curvesRoot 'build/curves-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/curves_fixture.py"
if($LASTEXITCODE -ne 0){throw 'Curve source audit failed'}
$curvesRuns=@()
$curvesCompilers=if($Sanitized){@('clang')}else{@('clang','gcc')}
$curvesOptimizations=if($Sanitized){@('O1')}else{@('O0','O2')}
foreach($curvesCompiler in $curvesCompilers) {
 foreach($curvesOptimization in $curvesOptimizations) {
  $curvesCpp=if($curvesCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($curvesShape in @('default','vanilla')) {
   $curvesFolder=Join-Path $curvesBuild $curvesShape
   $curvesArgs=@("-I$curvesFolder","-I$curvesBuild","-I$curvesRoot/src/control","-I$curvesRoot/src/core",'-include',"$curvesFolder/shape.h")
   if($Sanitized){$curvesArgs+=@('-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all')}
   & $curvesCompiler -std=c11 -pedantic-errors "-$curvesOptimization" @curvesArgs -c "$curvesRoot/src/control/Curves.c" -o "$curvesFolder/Curves.o"
   if($LASTEXITCODE -ne 0){throw 'C curve compile failed'}
   & $curvesCompiler -std=c11 -pedantic-errors "-$curvesOptimization" @curvesArgs -c "$curvesBuild/caller.c" -o "$curvesFolder/caller.o"
   if($LASTEXITCODE -ne 0){throw 'C curve caller failed'}
   & $curvesCpp -std=c++17 "-$curvesOptimization" @curvesArgs "$curvesBuild/before.cpp" -o "$curvesFolder/before.exe"
   if($LASTEXITCODE -ne 0){throw 'Original curve compile failed'}
   & $curvesCpp -std=c++17 "-$curvesOptimization" @curvesArgs "$curvesBuild/after.cpp" "$curvesFolder/Curves.o" "$curvesFolder/caller.o" -o "$curvesFolder/after.exe"
   if($LASTEXITCODE -ne 0){throw 'Mixed C curve link failed'}
   $curvesBefore=& "$curvesFolder/before.exe"
   if($LASTEXITCODE -ne 0){throw 'Original curve runtime failed'}
   $curvesAfter=& "$curvesFolder/after.exe"
   if($LASTEXITCODE -ne 0 -or $curvesBefore -ne $curvesAfter){throw "Curve comparison failed: $curvesBefore / $curvesAfter"}
   $curvesRuns += "$curvesCompiler-$curvesOptimization-$curvesShape : $curvesAfter"
   Write-Output $curvesRuns[-1]
  }
 }
}
$curvesResultsFile=if($Sanitized){'sanitized-runs.json'}else{'runtime-runs.json'}
$curvesRuns | ConvertTo-Json | Set-Content "$curvesBuild/$curvesResultsFile"
