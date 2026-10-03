$ErrorActionPreference = 'Stop'
$env:Path = 'C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$decoderRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$decoderBuild = Join-Path $decoderRoot 'build/audio-decoder-tests'
New-Item -ItemType Directory -Force -Path $decoderBuild | Out-Null
foreach ($decoderCompiler in @('clang', 'gcc')) {
    foreach ($decoderLanguage in @('c', 'c++')) {
        foreach ($decoderOptimization in @('O0', 'O2')) {
            $decoderName = "$decoderCompiler-$($decoderLanguage.Replace('+', 'p'))-$decoderOptimization"
            $decoderExe = Join-Path $decoderBuild "$decoderName.exe"
            $decoderStandard = if ($decoderLanguage -eq 'c') { '-std=c11' } else { '-std=c++17' }
            foreach ($decoderSource in @('audio_decoder', 'audio_adpcm')) {
                $decoderExe = Join-Path $decoderBuild "$decoderName-$decoderSource.exe"
                & $decoderCompiler -x $decoderLanguage $decoderStandard "-$decoderOptimization" -Wall -Wextra -Werror -pedantic (Join-Path $PSScriptRoot "$decoderSource.c") -o $decoderExe
                if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $decoderName-$decoderSource" }
                & $decoderExe
                if ($LASTEXITCODE -ne 0) { throw "Decoder test failed: $decoderName-$decoderSource" }
            }
        }
    }
}
$decoderPython = 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
$decoderFixture = Join-Path $decoderBuild 'decoder_fixture.cpp'
& $decoderPython (Join-Path $PSScriptRoot 'audio_decoder_fixture.py') $decoderFixture
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract production decoder implementations' }
foreach ($decoderCompiler in @('clang++', 'g++')) {
    foreach ($decoderOptimization in @('O0', 'O2')) {
        $decoderName = "$decoderCompiler-concrete-$decoderOptimization"
        $decoderExe = Join-Path $decoderBuild "$decoderName.exe"
        & $decoderCompiler -std=c++17 "-$decoderOptimization" -Wno-multichar "-I$decoderRoot" "-I$decoderRoot/src/audio/oal" "-I$decoderRoot/vendor/openal-soft/include" $decoderFixture -o $decoderExe
        if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $decoderName" }
        $decoderTranscript = & $decoderExe 'build/audio-decoder-tests'
        if ($LASTEXITCODE -ne 0) { throw "Concrete decoder test failed: $decoderName" }
        $decoderTranscript | Set-Content -LiteralPath (Join-Path $decoderBuild "$decoderName.txt")
        $decoderBaseline = Join-Path $decoderBuild 'baseline.txt'
        if (Test-Path -LiteralPath $decoderBaseline) {
            $decoderDifference = Compare-Object (Get-Content -LiteralPath $decoderBaseline) $decoderTranscript
            if ($decoderDifference) { throw "Decoder output differs from baseline: $decoderName" }
        }
        Write-Output "Concrete WAV/VB decode, seek, EOF and cleanup passed: $decoderName"
    }
}
