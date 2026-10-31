param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$shiny_textsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$shiny_textsPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $shiny_textsRoot
try {
    $shiny_textsExtra=@()
    if($Sanitized){$shiny_textsExtra+='--sanitized'}
    & $shiny_textsPython build/test_shiny_texts.py @shiny_textsExtra
    if($LASTEXITCODE -ne 0){throw 'Shiny texts comparison failed'}
    if($Native){
        & $shiny_textsPython build/test_shiny_texts_native.py
        if($LASTEXITCODE -ne 0){throw 'Native shiny_texts comparison failed'}
    }
    if($Consumers){
        & $shiny_textsPython build/check_shiny_texts_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual shiny_texts consumer compilation failed'}
    }
    if($BuildProject){
        & $shiny_textsPython build/build_shiny_texts.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $shiny_textsPython build/check_shiny_texts_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
