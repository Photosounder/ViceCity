param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$pedDataRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$pedDataPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $pedDataRoot
try {
    if($Sanitized){
        & $pedDataPython build/test_ped_data.py --sanitized
    }else{
        & $pedDataPython build/test_ped_data.py
    }
    if($LASTEXITCODE -ne 0){throw 'Pedestrian data comparison failed'}
    if($Native){
        & $pedDataPython build/test_ped_data_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native pedestrian data comparison failed'}
        & $pedDataPython build/check_ped_data_types.py
        if($LASTEXITCODE -ne 0){throw 'Native pedestrian data type checks failed'}
    }
    if($Consumers){
        & $pedDataPython build/check_ped_data_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual pedestrian data C compilation failed'}
    }
    if($BuildProject){
        & $pedDataPython build/build_ped_data.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $pedDataPython build/check_ped_data_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
