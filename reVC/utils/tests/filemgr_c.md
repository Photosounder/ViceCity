`CFileMgr` is removed. The complete file table, text adapter, directory management
and all 15 public operations are now C11 in `FileMgr.c`. The original one-argument
`OpenFile` becomes `FileMgr_OpenFileDefault`; it still selects binary-read mode.
The root getter returns the same live 128-byte buffer. All 401 code references in
46 caller files use the C API. The 50 original files, complete method/helper bodies,
headers and project entries are audited against immutable snapshots.

The signed `FileMgrSize` is exactly the original `ssize_t` type: `SSIZE_T` for MSVC,
`ptrdiff_t` for CodeWarrior and the platform `ssize_t` elsewhere. Native original
callers statically check the type equality. Reads retain their original const-qualified
buffer parameter and internal writable cast. The 19 usable slots, first-free-slot
selection, raw binary CRT mode, CRLF translation, embedded SUB bytes, true-on-failed-Seek
convention and EOF-based `GetErrorReadWrite` are unchanged. `LoadFile` retains its
fixed-size reads and original after-read assertion; this conversion does not repair
the preexisting capacity-check behavior. The development assertion calls `re3_assert`
and can continue if that handler returns; MASTER still evaluates the expression.
Diagnostic filenames, lines and function names naturally refer to the new C unit.

`FileMgr.cpp` is now a platform bridge: one folder query on Windows and two additional
case-path/file-open queries outside Windows. Case correction and platform-folder
implementations are still C++ elsewhere in the project. The Windows C kernel calls
`fopen` directly, as the original `fcaseopen` macro did.

The directory conversion's allocator audit exposed an earlier omission in the
non-Windows file-manager kernel: its casepath result was released with CRT free
after being allocated through CITAlloc in the C++ platform helper. FileMgr.c now
includes the actual CITAlloc header in that branch, retaining the original
release allocator and metadata. The strengthened simulated-POSIX comparison
uses actual CITAlloc declarations and guards the nine successful path
allocations. It verifies exact release pointers, mychdir metadata, intact guards
and matching allocation/release counts; a tenth path query returns NULL. Each
original/C run performs 149,554 checks and yields 9313abfb75eda2cc. This observes
the allocator boundary, without exercising CITAlloc's internal heap.

Quiet behavioral tests use actual CRT file reads/writes and directories exclusively
inside `build/filemgr-c-tests/run`. Each run makes more than 149,000 independent
checks. Coverage includes all byte values, CRLF/lone CR/NUL/SUB, line truncation and
EOF, length-one line buffers, six read chunk sizes, text and binary physical output,
all 19 slots and exhaustion/reuse, missing files, double close, seek/EOF behavior,
directory resets/extension, mutable root-pointer identity, the folder query's call
point, seven LoadFile lengths across 16 KiB boundaries and its existing assertions.
The original complete class/methods are compared with the actual C11 kernel, a
separately compiled C11 caller and the exact converted platform bridge.

Sixteen GCC/Clang O0/O2 comparisons cover default, no-FIX_BUGS, MASTER and simulated
POSIX branches. Four Clang O1 runs trap undefined behavior and float-cast overflow.
The simulated POSIX profile compiles the unchanged non-Windows C branches using
preincluded Windows CRT headers; its platform case-path helper is modeled, so this
does not establish a native Linux build or test the platform casepath algorithm.

Six Win32/Win64 comparisons use O2/GL/LTCG/OPT:REF and compile the original and new
platform bridge with the real project headers. All 100 default/native checks pass
(46 caller files, bridge and three kernel configurations on both architectures).
Header-only unused rouziclib interpreter references use the same abort-only fixture
definitions as the line tests; any invocation fails the test. Fixture linking
suppresses unused header auto-link requests for SDL2 and GLEW. Production build
dependencies and flags are unchanged.

Eight additional paired Win32/Win64 compilations pass for the GL3 GLFW source, GL3
custom pipeline, GL3 bridge (using the existing software-polygon configuration) and
enabled scene editor. The Win32 vanilla C kernel also compiles. Enabling PS2_MENU in
the complete PS2 frontend still produces the same 58 original errors before and
after this conversion on each architecture; it is not counted as a passing build.

The broader caller compilation exposed one missed date comparison in the frontend
resume-last-save loop. It now calls `Date_IsBefore` on the same two array entries.
The complete original selection loop and original date class are compared with the
new loop and actual Date.c on Win32/Win64 over 32,768 histories each. Invalid slot
zero and tie handling are preserved; both yield `bccf7fcb0cd3bff6 histories=32768`.

All 90 preceding whole-source audits pass through the new endpoint. Visual Studio
registers FileMgr.c as C11 and retains the platform bridge; CMake/Premake already
collect C sources. Run `utils/tests/test_filemgr.ps1`; switches `-Sanitized`, `-Native`,
`-Features` and `-AuditPrevious` select additional checks. Logs, original snapshots
and result records are in `build/filemgr-c-tests`.

These checks establish source, object and quiet file behavior. They do not establish
full game linking or gameplay, and no graphical tests were run.
