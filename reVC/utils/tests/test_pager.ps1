param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$pagerRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$pagerPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $pagerRoot
try {
    if($Sanitized){
        & $pagerPython build/test_pager.py --sanitized
    }else{
        & $pagerPython build/test_pager.py
    }
    if($LASTEXITCODE -ne 0){throw 'Pager comparison failed'}
    if($Native){
        & $pagerPython build/test_pager_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native pager comparison failed'}
        & $pagerPython build/check_pager_types.py
        if($LASTEXITCODE -ne 0){throw 'Native pager record checks failed'}
    }
    if($Consumers){
        & $pagerPython build/check_pager_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Pager consumer compilation failed'}
    }
    if($BuildProject){
        & $pagerPython build/build_pager.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $pagerPython build/check_pager_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
