`CIniFile` is removed. Its complete loader and two initialized float globals are
implemented in `IniFile.c` as C11. All 11 class references in five caller files
become the free C function or direct C-global reads/addresses. Density sliders
continue holding pointers to the same live multiplier values. Default limits and
initial multiplier values remain 25, 40, 12 and 0.6f.

The loader calls the converted file manager directly. It retains the original
directory reset, filename, text mode, two 200-byte line reads, scanf format,
ternary Min/Max clamps, close and three sequential integer assignments. Parse
failure preserves the current multiplier before clamping. EOF preserves stale
buffer text, so missing second lines can repeat the first parsed value. A missing
file leaves both multipliers unclamped but still applies their integer limits.
These behaviors and the float-to-integer truncation are deliberately unchanged.

The four functions remaining in IniFile.cpp return the existing shared gString
array and three live CPopulation/CCarCtrl integer addresses. The C loader does all
parsing, clamping, arithmetic and writes. No copied owner state or record layout
is introduced; these temporary typed readers remain C++ until their owners are
converted. The shared string address is obtained after a successful open and before
the first read, and its contents are neither cleared nor copied.

The original complete method is compared with the C11 implementation, a separate
C11 caller and the exact new bridge, using real CRT files and the actual C file
manager in an isolated build sandbox. Each run covers 4,495 cases and 34,451
independent checks, yielding `67dc13b77d5e2384 cases=4495 checks=34451`. Cases cover
static defaults, slider-pointer identity, missing/empty files, missing second lines,
parse failures, stale buffers, CRLF and embedded SUB, long lines split by the original
199-character capacity, extra ignored lines, CRT formats, overflow/infinity clamping,
signed zeros, finite clamp bounds, integer truncation and repeated loads.

Four Clang/GCC O0/O2 comparisons and two Win32/Win64 O2/GL/LTCG/OPT:REF comparisons
pass. Native original and new bridges use the actual project headers and owner
declarations. Sixteen native production compilation checks cover all five callers,
bridge, C unit and density sliders enabled/disabled on each architecture. One
Clang O1 run traps undefined behavior and float-cast overflow. Inputs that would
produce NaN or an unrepresentable float-to-int result are excluded because the
original integer assignment is undefined for those values; no new normalization
or exceptional-input behavior is introduced.

All 91 preceding whole-source migration audits pass. The older file-manager audit
recognizes the exact new four-reader host while the INI audit separately proves
the complete relocated algorithm. Nine existing source/project files and the two
new C/bridge-header files use LF line endings. Visual Studio explicitly compiles
IniFile.c as C11; CMake/Premake already collect C sources automatically.

Run `utils/tests/test_inifile.ps1`, adding `-Sanitized`, `-Native` or `-AuditPrevious`
for the corresponding checks. Models, immutable originals and logs are in
`build/inifile-c-tests`. Native fixture-only abort definitions satisfy unused
rouziclib header-table dependencies and fail if called; unused SDL2/GLEW auto-link
requests are suppressed only in fixture linking. Production dependencies are unchanged.

No graphical tests were run. Full game linking and gameplay remain unverified.
