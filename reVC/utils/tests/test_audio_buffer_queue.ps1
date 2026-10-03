$ErrorActionPreference = 'Stop'
$env:Path = 'C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$queueRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$queueBuild = Join-Path $queueRoot 'build/audio-buffer-queue-tests'
New-Item -ItemType Directory -Force -Path $queueBuild | Out-Null
foreach ($queueCompiler in @('clang', 'gcc')) {
    foreach ($queueLanguage in @('c', 'c++')) {
        foreach ($queueOptimization in @('O0', 'O2')) {
            $queueName = "$queueCompiler-$($queueLanguage.Replace('+', 'p'))-$queueOptimization"
            $queueExe = Join-Path $queueBuild "$queueName.exe"
            $queueStandard = if ($queueLanguage -eq 'c') { '-std=c11' } else { '-std=c++17' }
            & $queueCompiler -x $queueLanguage $queueStandard "-$queueOptimization" -Wall -Wextra -Werror -pedantic (Join-Path $PSScriptRoot 'audio_buffer_queue.c') -o $queueExe
            if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $queueName" }
            & $queueExe
            if ($LASTEXITCODE -ne 0) { throw "Queue test failed: $queueName" }
        }
    }
}
$queuePython = 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
$queueFixture = Join-Path $queueBuild 'stream_fixture.cpp'
& $queuePython (Join-Path $PSScriptRoot 'audio_stream_fixture.py') $queueFixture
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract production stream methods' }
foreach ($queueBackend in @('native', 'posix')) {
    $queueBackendArgs = @()
    if ($queueBackend -eq 'posix') { $queueBackendArgs += '-DAUDIO_THREAD_USE_POSIX' }
    foreach ($queueCompiler in @('clang++', 'g++')) {
        foreach ($queueOptimization in @('O0', 'O2')) {
            $queueName = "$queueBackend-$queueCompiler-worker-$queueOptimization"
            $queueExe = Join-Path $queueBuild "$queueName.exe"
            & $queueCompiler -std=c++17 "-$queueOptimization" -pthread @queueBackendArgs "-I$queueRoot" "-I$queueRoot/vendor/openal-soft/include" $queueFixture -o $queueExe
            if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $queueName" }
            & $queueExe
            if ($LASTEXITCODE -ne 0) { throw "Worker test failed: $queueName" }
        }
    }
}
