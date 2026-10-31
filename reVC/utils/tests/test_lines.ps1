param([switch]$Sanitized,[switch]$Native,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$linesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$linesPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $linesRoot
try {
    $linesExtra=@()
    if($Sanitized){$linesExtra+='--sanitized'}
    & $linesPython build/test_lines.py @linesExtra
    if($LASTEXITCODE -ne 0){throw 'Line behavior comparison failed'}
    if($Native){
        & $linesPython build/test_lines_native.py
        if($LASTEXITCODE -ne 0){throw 'Native line validation failed'}
    }
    if($AuditPrevious){
        & $linesPython build/check_lines_audits.py
        if($LASTEXITCODE -ne 0){throw 'Previous conversion source audit failed'}
    }
} finally { Pop-Location }
