param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$messagesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$messagesPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $messagesRoot
try {
    if($Sanitized){
        & $messagesPython build/test_messages.py --sanitized
    }else{
        & $messagesPython build/test_messages.py
    }
    if($LASTEXITCODE -ne 0){throw 'Message-manager comparison failed'}
    if($Native){
        & $messagesPython build/test_messages_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native message-manager comparison failed'}
        & $messagesPython build/check_messages_types.py
        if($LASTEXITCODE -ne 0){throw 'Native message-manager record checks failed'}
    }
    if($Consumers){
        & $messagesPython build/check_messages_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Message-manager consumer compilation failed'}
    }
    if($BuildProject){
        & $messagesPython build/build_messages.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $messagesPython build/check_messages_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
