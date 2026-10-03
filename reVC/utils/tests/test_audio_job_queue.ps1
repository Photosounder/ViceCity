$ErrorActionPreference = 'Stop'
$env:Path = 'C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$jobRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$jobBuild = Join-Path $jobRoot 'build/audio-scheduler-tests'
New-Item -ItemType Directory -Force -Path $jobBuild | Out-Null
foreach ($jobCompiler in @('clang', 'gcc')) {
    foreach ($jobLanguage in @('c', 'c++')) {
        foreach ($jobOptimization in @('O0', 'O2')) {
            $jobName = "$jobCompiler-$($jobLanguage.Replace('+', 'p'))-$jobOptimization"
            $jobExe = Join-Path $jobBuild "$jobName.exe"
            $jobStandard = if ($jobLanguage -eq 'c') { '-std=c11' } else { '-std=c++17' }
            & $jobCompiler -x $jobLanguage $jobStandard "-$jobOptimization" -Wall -Wextra -Werror -pedantic (Join-Path $PSScriptRoot 'audio_job_queue.c') -o $jobExe
            if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $jobName" }
            & $jobExe
            if ($LASTEXITCODE -ne 0) { throw "Queue test failed: $jobName" }
        }
    }
}
