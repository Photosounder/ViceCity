param([switch]$Sanitized,[switch]$Native,[switch]$Features,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$fileMgrRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$fileMgrPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $fileMgrRoot
try {
    $fileMgrExtra=@()
    if($Sanitized){$fileMgrExtra+='--sanitized'}
    & $fileMgrPython build/test_filemgr.py @fileMgrExtra
    if($LASTEXITCODE -ne 0){throw 'File-manager behavioral comparison failed'}
    if($Native){
        & $fileMgrPython build/test_filemgr_native.py
        if($LASTEXITCODE -ne 0){throw 'Native file-manager validation failed'}
        & $fileMgrPython build/test_filemgr_date.py
        if($LASTEXITCODE -ne 0){throw 'Resume-save date regression failed'}
    }
    if($Features){
        & $fileMgrPython build/check_filemgr_features.py
        if($LASTEXITCODE -ne 0){throw 'File-manager feature compilation comparison failed'}
    }
    if($AuditPrevious){
        & $fileMgrPython build/check_filemgr_audits.py
        if($LASTEXITCODE -ne 0){throw 'Previous conversion source audit failed'}
    }
} finally { Pop-Location }
