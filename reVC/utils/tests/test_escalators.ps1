param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$escalatorsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$escalatorsPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $escalatorsRoot
try {
    $escalatorsExtra=@()
    if($Sanitized){$escalatorsExtra+='--sanitized'}
    & $escalatorsPython build/test_escalators.py @escalatorsExtra
    if($LASTEXITCODE -ne 0){throw 'Escalator-controller comparison failed'}
    if($Native){
        & $escalatorsPython build/test_escalators_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native escalator-controller comparison failed'}
        & $escalatorsPython build/check_escalators_types.py
        if($LASTEXITCODE -ne 0){throw 'Native escalator-controller type checks failed'}
    }
    if($Consumers){
        & $escalatorsPython build/check_escalators_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual escalator-controller consumer compilation failed'}
    }
    if($BuildProject){
        & $escalatorsPython build/build_escalators.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $escalatorsPython build/check_escalators_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
