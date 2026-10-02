$ErrorActionPreference = 'Stop'
$env:Path = 'C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$saveRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$saveTestDir = Join-Path $saveRoot 'build/savebuf-tests'
New-Item -ItemType Directory -Force -Path $saveTestDir | Out-Null

# Route assertions to a quiet exit so malformed-header tests cannot open a CRT dialog
$saveAssertHeader = @'
#ifndef SAVE_BUFFER_TEST_ASSERT_H
#define SAVE_BUFFER_TEST_ASSERT_H
#include <stdio.h>
#include <stdlib.h>
static inline void save_buffer_assert(int passed, const char *expression)
{
    // Stop on failed assertions without invoking the Windows CRT dialog
    if (!passed) {
        // Identify the invalid header check and return a recognizable status
        fprintf(stderr, "Assertion failed: %s\n", expression);
        exit(71);
    }
}
#define assert(expression) save_buffer_assert(!!(expression), #expression)
#endif
'@
[IO.File]::WriteAllText((Join-Path $saveTestDir 'assert.h'), $saveAssertHeader.Replace("`r`n", "`n"))

$saveRuns = 0
foreach ($saveCompiler in @('clang', 'gcc')) {
    foreach ($saveLanguage in @('c', 'c++')) {
        foreach ($saveOptimization in @('O0', 'O2')) {
            foreach ($saveValidation in @(0, 1)) {
                foreach ($saveCompatibility in @(0, 1)) {
                    foreach ($saveCustomAssert in @(0, 1)) {
                        $saveName = "$saveCompiler-$($saveLanguage.Replace('+', 'p'))-$saveOptimization-$saveValidation-$saveCompatibility-$saveCustomAssert"
                        $saveExecutable = Join-Path $saveTestDir "$saveName.exe"
                        $saveStandard = if ($saveLanguage -eq 'c') { '-std=c11' } else { '-std=c++17' }
                        $saveArgs = @('-x', $saveLanguage, $saveStandard, "-$saveOptimization", '-Wall', '-Wextra', '-Werror', '-pedantic', "-I$saveTestDir")
                        if ($saveValidation) { $saveArgs += '-DVALIDATE_SAVE_SIZE' }
                        if ($saveCompatibility) { $saveArgs += '-DCOMPATIBLE_SAVES' }
                        if ($saveCustomAssert) { $saveArgs += '-DSAVE_BUFFER_PREDEFINED_ASSERT' }
                        $saveArgs += @((Join-Path $PSScriptRoot 'savebuf.c'), '-o', $saveExecutable)
                        & $saveCompiler @saveArgs
                        if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $saveName" }
                        & $saveExecutable | Out-Null
                        if ($LASTEXITCODE -ne 0) { throw "Buffer test failed: $saveName" }
                        foreach ($saveCorruption in @('magic', 'size', 'magic-length', 'size-length')) {
                            $saveOutput = & $saveExecutable $saveCorruption 2>&1
                            $saveExpectedExit = if ($saveValidation) { 71 } else { 0 }
                            if ($LASTEXITCODE -ne $saveExpectedExit) { throw "Header test failed: $saveName $saveCorruption`n$saveOutput" }
                        }
                        $saveRuns++
                    }
                }
            }
        }
    }
}
Write-Output "Save buffer tests passed in $saveRuns compiler/language/optimization/configuration combinations"
