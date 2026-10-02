$ErrorActionPreference = 'Stop'
$env:Path = 'C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$poolRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$poolTestDir = Join-Path $poolRoot 'build/pool-store-tests'
New-Item -ItemType Directory -Force -Path $poolTestDir | Out-Null
$poolRuns = 0
foreach ($poolCompiler in @('clang', 'gcc')) {
    foreach ($poolLanguage in @('c', 'c++')) {
        foreach ($poolOptimization in @('O0', 'O2')) {
            foreach ($poolFixes in @(0, 1)) {
                foreach ($poolCustomAssert in @(0, 1)) {
                    $poolName = "$poolCompiler-$($poolLanguage.Replace('+', 'p'))-$poolOptimization-$poolFixes-$poolCustomAssert"
                    $poolExecutable = Join-Path $poolTestDir "$poolName.exe"
                    $poolStandard = if ($poolLanguage -eq 'c') { '-std=c11' } else { '-std=c++17' }
                    $poolArgs = @('-x', $poolLanguage, $poolStandard, "-$poolOptimization", '-Wall', '-Wextra', '-Werror', '-pedantic')
                    if ($poolFixes) { $poolArgs += '-DFIX_BUGS' }
                    if ($poolCustomAssert) { $poolArgs += '-DPOOL_PREDEFINED_ASSERT' }
                    $poolArgs += @((Join-Path $PSScriptRoot 'pool_store.c'), '-o', $poolExecutable)
                    & $poolCompiler @poolArgs
                    if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $poolName" }
                    & $poolExecutable | Out-Null
                    if ($LASTEXITCODE -ne 0) { throw "Pool or store test failed: $poolName" }
                    $poolRuns++
                }
            }
        }
    }
}
Write-Output "Pool and store tests passed in $poolRuns compiler/language/optimization/configuration combinations"
