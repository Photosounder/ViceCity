param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$gangsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$gangsPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $gangsRoot
try {
    if($Sanitized){
        & $gangsPython build/test_gangs.py --sanitized
    }else{
        & $gangsPython build/test_gangs.py
    }
    if($LASTEXITCODE -ne 0){throw 'Gang data comparison failed'}
    if($Native){
        & $gangsPython build/test_gangs_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native gang data comparison failed'}
        & $gangsPython build/check_gangs_types.py
        if($LASTEXITCODE -ne 0){throw 'Native gang data type checks failed'}
    }
    if($Consumers){
        & $gangsPython build/check_gangs_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual gang data C compilation failed'}
    }
    if($BuildProject){
        & $gangsPython build/build_gangs.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $gangsPython build/check_gangs_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
