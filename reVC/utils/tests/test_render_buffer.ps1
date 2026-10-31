param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$render_bufferRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$render_bufferPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $render_bufferRoot
try {
    $render_bufferExtra=@()
    if($Sanitized){$render_bufferExtra+='--sanitized'}
    & $render_bufferPython build/test_render_buffer.py @render_bufferExtra
    if($LASTEXITCODE -ne 0){throw 'Render buffer comparison failed'}
    if($Native){
        & $render_bufferPython build/test_render_buffer_native.py
        if($LASTEXITCODE -ne 0){throw 'Native render_buffer comparison failed'}
    }
    if($Consumers){
        & $render_bufferPython build/check_render_buffer_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual render_buffer consumer compilation failed'}
    }
    if($BuildProject){
        & $render_bufferPython build/build_render_buffer.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $render_bufferPython build/check_render_buffer_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
