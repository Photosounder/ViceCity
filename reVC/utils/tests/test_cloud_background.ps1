param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$cloudBackgroundRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$cloudBackgroundPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $cloudBackgroundRoot
try {
    $cloudBackgroundExtra=@()
    if($Sanitized){$cloudBackgroundExtra+='--sanitized'}
    & $cloudBackgroundPython build/test_cloud_background.py @cloudBackgroundExtra
    if($LASTEXITCODE -ne 0){throw 'Cloud background comparison failed'}
    if($Native){
        & $cloudBackgroundPython build/test_cloud_background_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native cloud background comparison failed'}
        & $cloudBackgroundPython build/check_cloud_background_types.py
        if($LASTEXITCODE -ne 0){throw 'Native cloud background type checks failed'}
    }
    if($Consumers){
        & $cloudBackgroundPython build/check_cloud_background_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual cloud background consumer compilation failed'}
    }
    if($BuildProject){
        & $cloudBackgroundPython build/build_cloud_background.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $cloudBackgroundPython build/check_cloud_background_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
