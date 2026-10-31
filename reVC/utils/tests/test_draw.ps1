param([switch]$Sanitized,[switch]$Native,[switch]$AuditPrevious,[switch]$BroadConsumers)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$drawRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$drawPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $drawRoot
try {
    $drawExtra=@()
    if($Sanitized){$drawExtra+='--sanitized'}
    & $drawPython build/test_draw.py @drawExtra
    if($LASTEXITCODE -ne 0){throw 'Drawing comparison failed'}
    if($Native){
        & $drawPython build/test_draw_native.py
        if($LASTEXITCODE -ne 0){throw 'Native drawing validation failed'}
    }
    if($BroadConsumers){
        & $drawPython build/compile_general_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Broad common-header consumer compilation failed'}
    }
    if($AuditPrevious){
        & $drawPython build/check_draw_audits.py
        if($LASTEXITCODE -ne 0){throw 'Previous migration source audit failed'}
    }
} finally { Pop-Location }
