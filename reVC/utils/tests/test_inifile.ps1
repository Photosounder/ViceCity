param([switch]$Sanitized,[switch]$Native,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$iniFileRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$iniFilePython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $iniFileRoot
try {
    $iniFileExtra=@()
    if($Sanitized){$iniFileExtra+='--sanitized'}
    & $iniFilePython build/test_inifile.py @iniFileExtra
    if($LASTEXITCODE -ne 0){throw 'INI behavioral comparison failed'}
    if($Native){
        & $iniFilePython build/test_inifile_native.py
        if($LASTEXITCODE -ne 0){throw 'Native INI validation failed'}
    }
    if($AuditPrevious){
        & $iniFilePython build/check_inifile_audits.py
        if($LASTEXITCODE -ne 0){throw 'Previous migration source audit failed'}
    }
} finally { Pop-Location }
