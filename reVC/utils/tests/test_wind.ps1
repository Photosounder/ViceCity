param([switch]$Sanitized,[switch]$Native,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$windRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$windPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $windRoot
try {
    $windExtra=@()
    if($Sanitized){$windExtra+='--sanitized'}
    & $windPython build/test_wind.py @windExtra
    if($LASTEXITCODE -ne 0){throw 'Wind behavioral comparison failed'}
    if($Native){
        & $windPython build/test_wind_native.py
        if($LASTEXITCODE -ne 0){throw 'Native wind validation failed'}
    }
    if($AuditPrevious){
        & $windPython build/check_wind_audits.py
        if($LASTEXITCODE -ne 0){throw 'Previous migration source audit failed'}
    }
} finally { Pop-Location }
