$ErrorActionPreference = 'Stop'
$env:Path = 'C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$threadRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$threadBuild = Join-Path $threadRoot 'build/audio-thread-tests'
New-Item -ItemType Directory -Force -Path $threadBuild | Out-Null
foreach ($threadBackend in @('native', 'posix')) {
    $threadBackendArgs = @()
    if ($threadBackend -eq 'posix') { $threadBackendArgs += '-DAUDIO_THREAD_USE_POSIX' }
    foreach ($threadCompiler in @('clang', 'gcc')) {
        foreach ($threadLanguage in @('c', 'c++')) {
            foreach ($threadOptimization in @('O0', 'O2')) {
                $threadName = "$threadBackend-$threadCompiler-$($threadLanguage.Replace('+', 'p'))-$threadOptimization"
                $threadExe = Join-Path $threadBuild "$threadName.exe"
                $threadStandard = if ($threadLanguage -eq 'c') { '-std=c11' } else { '-std=c++17' }
                & $threadCompiler -x $threadLanguage $threadStandard "-$threadOptimization" -pthread -Wall -Wextra -Werror -pedantic @threadBackendArgs (Join-Path $PSScriptRoot 'audio_thread.c') -o $threadExe
                if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $threadName" }
                & $threadExe
                if ($LASTEXITCODE -ne 0) { throw "Thread test failed: $threadName" }
            }
        }
    }
}
