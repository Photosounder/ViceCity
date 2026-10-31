param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$markerRecordRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$markerRecordPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $markerRecordRoot
try {
    $markerRecordExtra=@()
    if($Sanitized){$markerRecordExtra+='--sanitized'}
    & $markerRecordPython build/test_marker_record.py @markerRecordExtra
    if($LASTEXITCODE -ne 0){throw 'Marker record comparison failed'}
    if($Native){
        & $markerRecordPython build/test_marker_record_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native marker record comparison failed'}
    }
    if($Consumers){
        & $markerRecordPython build/check_marker_record_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual marker consumer compilation failed'}
    }
    if($BuildProject){
        & $markerRecordPython build/build_marker_record.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $markerRecordPython build/check_marker_record_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
