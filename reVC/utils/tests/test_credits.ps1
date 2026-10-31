param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$creditsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$creditsPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $creditsRoot
try {
    $creditsExtra=@()
    if($Sanitized){$creditsExtra+='--sanitized'}
    & $creditsPython build/test_credits.py @creditsExtra
    if($LASTEXITCODE -ne 0){throw 'Credits comparison failed'}
    if($Native){
        & $creditsPython build/test_credits_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native credits comparison failed'}
    }
    if($Consumers){
        & $creditsPython build/check_credits_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual credits consumer compilation failed'}
    }
    if($BuildProject){
        & $creditsPython build/build_credits.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $creditsPython build/check_credits_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
