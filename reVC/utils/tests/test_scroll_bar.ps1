param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$scrollBarRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$scrollBarPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $scrollBarRoot
try {
    $scrollBarExtra=@()
    if($Sanitized){$scrollBarExtra+='--sanitized'}
    & $scrollBarPython build/test_scroll_bar.py @scrollBarExtra
    if($LASTEXITCODE -ne 0){throw 'Scrolling-sign comparison failed'}
    if($Native){
        & $scrollBarPython build/test_scroll_bar_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native scrolling-sign comparison failed'}
        & $scrollBarPython build/check_scroll_bar_types.py
        if($LASTEXITCODE -ne 0){throw 'Native scrolling-sign type checks failed'}
    }
    if($Consumers){
        & $scrollBarPython build/check_scroll_bar_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual scrolling-sign consumer compilation failed'}
    }
    if($BuildProject){
        & $scrollBarPython build/build_scroll_bar.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $scrollBarPython build/check_scroll_bar_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
