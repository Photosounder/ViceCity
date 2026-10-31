param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$escalatorRecordRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$escalatorRecordPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $escalatorRecordRoot
try {
    $escalatorRecordExtra=@()
    if($Sanitized){$escalatorRecordExtra+='--sanitized'}
    & $escalatorRecordPython build/test_escalator_record.py @escalatorRecordExtra
    if($LASTEXITCODE -ne 0){throw 'Escalator-record comparison failed'}
    if($Native){
        & $escalatorRecordPython build/test_escalator_record_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native escalator-record comparison failed'}
        & $escalatorRecordPython build/check_escalator_record_types.py
        if($LASTEXITCODE -ne 0){throw 'Native escalator-record type checks failed'}
    }
    if($Consumers){
        & $escalatorRecordPython build/check_escalator_record_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual escalator-record consumer compilation failed'}
    }
    if($BuildProject){
        & $escalatorRecordPython build/build_escalator_record.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $escalatorRecordPython build/check_escalator_record_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
