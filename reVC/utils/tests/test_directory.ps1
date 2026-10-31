param([switch]$Sanitized,[switch]$Native,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$directoryRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$directoryPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $directoryRoot
try {
    $directoryExtra=@()
    if($Sanitized){$directoryExtra+='--sanitized'}
    & $directoryPython build/test_directory.py @directoryExtra
    if($LASTEXITCODE -ne 0){throw 'Directory behavioral comparison failed'}
    if($Native){
        & $directoryPython build/test_directory_native.py
        if($LASTEXITCODE -ne 0){throw 'Native directory validation failed'}
    }
    if($AuditPrevious){
        & $directoryPython build/check_directory_audits.py
        if($LASTEXITCODE -ne 0){throw 'Previous migration source audit failed'}
    }
} finally { Pop-Location }
