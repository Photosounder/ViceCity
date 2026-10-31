param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$money_messagesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$money_messagesPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $money_messagesRoot
try {
    $money_messagesExtra=@()
    if($Sanitized){$money_messagesExtra+='--sanitized'}
    & $money_messagesPython build/test_money_messages.py @money_messagesExtra
    if($LASTEXITCODE -ne 0){throw 'Money messages comparison failed'}
    if($Native){
        & $money_messagesPython build/test_money_messages_native.py
        if($LASTEXITCODE -ne 0){throw 'Native money_messages comparison failed'}
    }
    if($Consumers){
        & $money_messagesPython build/check_money_messages_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual money_messages consumer compilation failed'}
    }
    if($BuildProject){
        & $money_messagesPython build/build_money_messages.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $money_messagesPython build/check_money_messages_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
