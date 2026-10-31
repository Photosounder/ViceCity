param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$userDisplayRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$userDisplayPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $userDisplayRoot
try {
    if($Sanitized){
        & $userDisplayPython build/test_user_display.py --sanitized
    }else{
        & $userDisplayPython build/test_user_display.py
    }
    if($LASTEXITCODE -ne 0){throw 'User-display comparison failed'}
    if($Native){
        & $userDisplayPython build/test_user_display_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native user-display comparison failed'}
        & $userDisplayPython build/check_user_display_types.py
        if($LASTEXITCODE -ne 0){throw 'Native user-display record checks failed'}
    }
    if($Consumers){
        & $userDisplayPython build/check_user_display_consumers.py
        if($LASTEXITCODE -ne 0){throw 'User-display consumer compilation failed'}
    }
    if($BuildProject){
        & $userDisplayPython build/build_user_display.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $userDisplayPython build/check_user_display_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
