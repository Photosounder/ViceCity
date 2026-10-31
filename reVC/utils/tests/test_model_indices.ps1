param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$modelIndicesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$modelIndicesPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $modelIndicesRoot
try {
    $modelIndicesExtra=@()
    if($Sanitized){$modelIndicesExtra+='--sanitized'}
    & $modelIndicesPython build/test_model_indices.py @modelIndicesExtra
    if($LASTEXITCODE -ne 0){throw 'Model registry comparison failed'}
    if($Native){
        & $modelIndicesPython build/test_model_indices_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native model registry comparison failed'}
        & $modelIndicesPython build/check_model_indices_types.py
        if($LASTEXITCODE -ne 0){throw 'Native model registry type checks failed'}
    }
    if($Consumers){
        & $modelIndicesPython build/check_model_indices_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual model registry consumer compilation failed'}
    }
    if($BuildProject){
        & $modelIndicesPython build/build_model_indices.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $modelIndicesPython build/check_model_indices_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
