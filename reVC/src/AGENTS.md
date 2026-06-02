# Local Tooling

Use MSYS2 tools from `C:\msys\ucrt64\bin` and `C:\msys\usr\bin`, clang, gcc, and llvm-mca are available there.

When running compiler or LLVM tools from PowerShell, prepend:

```powershell
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
```

# Edit Markers

Mark all Codex edits with either an inline `// rouz edit (ChatGPT)` for single-line edits or `//+ rouz edit (ChatGPT)` and `//- rouz edit (ChatGPT)` before and after multi-line edits.

# Line endings

If you encounter a file with CRLF line endings, convert the whole file to LF.
