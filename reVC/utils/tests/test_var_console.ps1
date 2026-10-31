param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$varConsoleRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$varConsolePython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $varConsoleRoot
try {
    if($Sanitized){
        & $varConsolePython build/test_var_console.py --sanitized
    }else{
        & $varConsolePython build/test_var_console.py
    }
    if($LASTEXITCODE -ne 0){throw 'Variable-console comparison failed'}
    if($Native){
        & $varConsolePython build/test_var_console_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native variable-console comparison failed'}
        & $varConsolePython build/check_var_console_types.py
        if($LASTEXITCODE -ne 0){throw 'Native variable-console type checks failed'}
    }
    if($Consumers){
        & $varConsolePython build/check_var_console_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual variable-console C compilation failed'}
    }
    if($BuildProject){
        & $varConsolePython build/build_var_console.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $varConsolePython build/check_var_console_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
