param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$skidmarkRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$skidmarkPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $skidmarkRoot
try {
    $skidmarkExtra=@()
    if($Sanitized){$skidmarkExtra+='--sanitized'}
    & $skidmarkPython build/test_skidmarks.py @skidmarkExtra
    if($LASTEXITCODE -ne 0){throw 'Skidmark comparison failed'}
    if($Native){
        & $skidmarkPython build/test_skidmarks_native.py
        if($LASTEXITCODE -ne 0){throw 'Native skidmark comparison failed'}
    }
    if($Consumers){
        & $skidmarkPython build/check_skidmarks_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual skidmark consumer compilation failed'}
    }
    if($BuildProject){
        & $skidmarkPython build/build_skidmarks.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $skidmarkPython build/check_skidmarks_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
