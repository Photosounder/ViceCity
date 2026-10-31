param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$scriptPathsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$scriptPathsPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $scriptPathsRoot
try {
    $scriptPathsExtra=@()
    if($Sanitized){$scriptPathsExtra+='--sanitized'}
    & $scriptPathsPython build/test_script_paths.py @scriptPathsExtra
    if($LASTEXITCODE -ne 0){throw 'Scripted-path comparison failed'}
    if($Native){
        & $scriptPathsPython build/test_script_paths_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native scripted-path comparison failed'}
        & $scriptPathsPython build/check_script_paths_types.py
        if($LASTEXITCODE -ne 0){throw 'Native scripted-path type checks failed'}
    }
    if($Consumers){
        & $scriptPathsPython build/check_script_paths_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual scripted-path consumer compilation failed'}
    }
    if($BuildProject){
        & $scriptPathsPython build/build_script_paths.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $scriptPathsPython build/check_script_paths_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
