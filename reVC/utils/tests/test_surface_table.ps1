param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$surfaceTableRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$surfaceTablePython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $surfaceTableRoot
try {
    $surfaceTableExtra=@()
    if($Sanitized){$surfaceTableExtra+='--sanitized'}
    & $surfaceTablePython build/test_surface_table.py @surfaceTableExtra
    if($LASTEXITCODE -ne 0){throw 'Surface properties comparison failed'}
    if($Native){
        & $surfaceTablePython build/test_surface_table_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native surface properties comparison failed'}
        & $surfaceTablePython build/check_surface_table_types.py
        if($LASTEXITCODE -ne 0){throw 'Native surface properties type checks failed'}
    }
    if($Consumers){
        & $surfaceTablePython build/check_surface_table_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual surface properties consumer compilation failed'}
    }
    if($BuildProject){
        & $surfaceTablePython build/build_surface_table.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $surfaceTablePython build/check_surface_table_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
