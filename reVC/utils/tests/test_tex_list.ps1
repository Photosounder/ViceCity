param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$texListRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$texListPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $texListRoot
try {
    if($Sanitized){
        & $texListPython build/test_tex_list.py --sanitized
    }else{
        & $texListPython build/test_tex_list.py
    }
    if($LASTEXITCODE -ne 0){throw 'Texture-list comparison failed'}
    if($Native){
        & $texListPython build/test_tex_list_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native texture-list comparison failed'}
        & $texListPython build/check_tex_list_types.py
        if($LASTEXITCODE -ne 0){throw 'Native texture-list type checks failed'}
    }
    if($Consumers){
        & $texListPython build/check_tex_list_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual texture-list C compilation failed'}
    }
    if($BuildProject){
        & $texListPython build/build_tex_list.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $texListPython build/check_tex_list_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
