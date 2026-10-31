param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$consoleRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$consolePython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $consoleRoot
try {
    $consoleExtra=@()
    if($Sanitized){$consoleExtra+='--sanitized'}
    & $consolePython build/test_console.py @consoleExtra
    if($LASTEXITCODE -ne 0){throw 'Console comparison failed'}
    if($Native){
        & $consolePython build/test_console_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native console comparison failed'}
    }
    if($Consumers){
        & $consolePython build/check_console_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual console consumer compilation failed'}
    }
    if($BuildProject){
        & $consolePython build/build_console.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $consolePython build/check_console_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
