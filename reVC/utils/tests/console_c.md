Console.c removes the CConsole class and owns the complete Console C record,
TheConsole singleton, four instance operations and existing cprintf variadic
formatting. The public header is valid C11 and uses explicit Console pointers.
There is no C++ console storage or member function left. Game initialization and
display callers use the C functions. Fourteen typed font boundary operations
retain the exact existing color construction and mutable Unicode print overload.

Native Win32/Win64 probes compare every original field type and offset, complete
size, alignment, ring/row dimensions, wchar/uint16_t identity and variadic/font
signatures. Both the actual original class and new C record occupy 700 bytes.
The C compiler independently asserts that record width. All Unicode rows,
timestamps and color arrays remain in the canonical C record. Init resets only
the original two counters and retains all other elements and metadata.

AddLine retains the strict length-greater-than-forty split, forty-byte temporary
copy, terminator at element thirty-nine and thirty-nine-byte source advance.
AddOneLine retains direct byte-to-Unicode conversion, the repeated source read
after each destination write, untouched tail elements, timer/color publication
order and exact ring-count behavior. The original normal seven-visible-line
behavior is preserved. Both signed and unsigned plain-char profiles are compared.
Legal character aliases into actual Unicode rows are borrowed directly, including
overlapping reads/writes at offsets zero, one and two; no source snapshot is made.

Display preserves all original font-state writes before expiration, including
the unfixed proportional-font override. Expiration uses unsigned subtraction and
the strict greater-than-20000 test. The loop's count/current-line reads remain
live after owner callbacks; each iteration retains its captured row index. Font
prints borrow actual C row addresses, so text/color mutations, insertions,
partial Init calls and nested displays retain original behavior. cprintf retains
the complete original formatting and singleton insertion sequence; test formats
remain within its existing local buffer capacity.

console_fixture.py audits six complete changed source/project files, both new
files, every record field, the four methods and the complete variadic function.
The oracle compiles the original three out-of-line methods, inline Init and
cprintf statements unchanged. An access-only header adjustment permits observers
to seed/read original private fields. Observations copy declared fields through
their actual types, never reinterpret a C++ record as a C record. Two extra real
instances test independent storage; the new variant owns those instances in C.
Separate C11 callers exercise the complete public API and variadic entry.

Each comparison observes 2,074 complete three-instance snapshots, 1,731 insertions,
14,798 string prints, 1,649 displays and three variadic-format cases. Cases cover
all split/truncation boundaries, all 255 nonzero byte values, retained Unicode
tails, ring saturation/current indices, exact expiration boundaries, timer wrap,
overlapping borrowed inputs, promoted variadic types, live counter/color/row/
timer/viewport changes and nested rendering. A further 1,500 mixed frames exercise
insertion, partial resets and expiration. Default fingerprint: 957c86cba51e17ea.

Sixteen Clang/GCC O0/O2 comparisons and four trap sanitizer comparisons cover
librw, non-librw, unfixed and unsigned-char profiles. Five MSVC/LTCG comparisons
cover Win32/Win64 D3D9/GL3 plus vanilla Win32. All match the original. Twenty-five
actual production compilations and both Debug/Release x64 D3D9/OpenAL builds pass.
No game is launched. All fifty project configurations retain distinct objects
for twenty-seven C/C++ pairs, including Console.c.obj and Console.cpp.obj. All 113
preceding whole-source audits pass. Immutable originals and evidence are under
build/console-c-tests; test_console.ps1 runs quiet checks and
build/finalize_console.py verifies recorded final evidence.
