param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$rubbishRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$rubbishPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $rubbishRoot
try {
    $rubbishExtra=@()
    if($Sanitized){$rubbishExtra+='--sanitized'}
    & $rubbishPython build/test_rubbish.py @rubbishExtra
    if($LASTEXITCODE -ne 0){throw 'Rubbish comparison failed'}
    if($Native){
        & $rubbishPython build/test_rubbish_native.py
        if($LASTEXITCODE -ne 0){throw 'Native rubbish comparison failed'}
    }
    if($Consumers){
        & $rubbishPython build/check_rubbish_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual rubbish consumer compilation failed'}
    }
    if($BuildProject){
        & $rubbishPython build/build_rubbish.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $rubbishPython build/check_rubbish_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
