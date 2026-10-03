$ErrorActionPreference = 'Stop'
$env:Path = 'C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$formatRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$formatBuild = Join-Path $formatRoot 'build/audio-format-tests'
New-Item -ItemType Directory -Force -Path $formatBuild | Out-Null
foreach ($formatCompiler in @('clang', 'gcc')) {
    foreach ($formatLanguage in @('c', 'c++')) {
        foreach ($formatOptimization in @('O0', 'O2')) {
            $formatName = "$formatCompiler-$($formatLanguage.Replace('+', 'p'))-$formatOptimization"
            $formatExe = Join-Path $formatBuild "$formatName.exe"
            $formatStandard = if ($formatLanguage -eq 'c') { '-std=c11' } else { '-std=c++17' }
            & $formatCompiler -x $formatLanguage $formatStandard "-$formatOptimization" -Wall -Wextra -Werror -pedantic -Wno-multichar (Join-Path $PSScriptRoot 'audio_formats.c') -o $formatExe
            if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $formatName" }
            & $formatExe 'build/audio-format-tests'
            if ($LASTEXITCODE -ne 0) { throw "Format test failed: $formatName" }
        }
    }
}
$formatParseArgs = @('-fsyntax-only', '-Wall', '-Wextra', '-Werror', '-pedantic', '-Wno-multichar', '-Wno-unused-function', '-Wno-unused-const-variable',
    "-I$formatRoot/src/audio/oal", "-I$formatRoot/vendor/libsndfile/include", "-I$formatRoot/vendor/mpg123/include", "-I$formatRoot/vendor/opusfile/include", "-I$formatRoot/vendor/opus/include", "-I$formatRoot/vendor/ogg/include")
foreach ($formatCompiler in @('clang', 'gcc')) {
    foreach ($formatLanguage in @('c', 'c++')) {
        $formatStandard = if ($formatLanguage -eq 'c') { '-std=c11' } else { '-std=c++17' }
        foreach ($formatBits in @('64', '32')) {
            $formatBitsArg = if ($formatBits -eq '32') { '-m32' } else { '-m64' }
            & $formatCompiler -x $formatLanguage $formatStandard $formatBitsArg @formatParseArgs -DAUDIO_OAL_USE_SNDFILE -DAUDIO_OAL_USE_MPG123 -DAUDIO_OAL_USE_OPUS (Join-Path $PSScriptRoot 'audio_formats.c')
            if ($LASTEXITCODE -ne 0) { throw "All-format syntax failed: $formatCompiler-$formatLanguage-$formatBits" }
        }
    }
}
Write-Output 'Eight strict C/C++ optional-format checks passed'
