# C animation links and clump data

`AnimBlendList.c` owns the two-pointer animation link and all three operations.
`AnimBlendClumpData.c` owns the complete clump record and five operations:
initialization, cleanup, frame replacement, the bone-count alias and frame
traversal. Both C++ classes, their member methods and the clump constructor and
destructor are removed. Native plugin allocation still uses the same `malloc`
and `free`; explicit C initialization and cleanup replace allocator construction
and destructor calls at the same points.

Links remain 8 bytes on x86 and 16 bytes on x64. Clump records remain 20 and 40
bytes, with unchanged link, count, velocity union and frame-pointer offsets.
The unchanged SDK frame record lives in `AnimBlendFrameData_native.h` and remains
24/32 bytes. It is opaque to C. Native compiler-emitted COFF constants compare
association, update-data and frame sizes/offsets before and after in every x86/
x64 collision profile. The C clump header retains the SDK record for native C++
consumers and opaque native vector/frame pointers for C callers.

Link operations preserve every original live read and pointer write, including
self-insertion, cycles and already-linked aliases. Removal updates the previous
neighbor, then the next neighbor, then clears its own fields. Clump cleanup
detaches its link before freeing frames. Frame replacement frees old storage
before storing the new signed count and allocating the replacement with 64-byte
alignment. The original size expression is narrowed to the native allocator's
unsigned 32-bit argument after multiplication. Negative and large counts retain
their original argument conversion; no new validation policy is added.

Frame traversal re-reads both count and storage after every callback. Callbacks
can shrink, grow or replace the live container. Three native primitives expose
only the actual SDK frame stride and existing aligned allocation/free functions.
All lifetime, list and traversal algorithms are C. The native plugin still removes
associations before clump cleanup and releases raw clump storage afterward.

The independent C11 fixture compares actual original methods with C over 74,754
states. It exhausts all three-node next/previous graphs and receiver/argument
aliases (61,440 operations), performs 8,192 live-list updates, checks 2,048 signed
allocation-boundary cases and 2,048 callback mutation scenarios, and executes the
actual native plugin lifetime bodies across 1,024 cases. Four actual native frame
callbacks verify initialization, frame lookup and hierarchy-array filling.
Allocation requests are recorded while the fixture allocator bounds actual test
storage, so large numeric argument tests do not request huge allocations.

Constructor comparisons cover all initialized fields. They deliberately exclude
padding that the original C++ lifetime leaves unspecified. A diagnostic proved
that GCC O2 clears only the four x64 padding bytes between count and velocity;
requiring those bytes to retain an earlier storage pattern was an invalid fixture
assumption. Link identities are normalized, and no native addresses enter hashes.
Pointer updates, callback order, allocator argument widths and visible fields are
compared exactly. All allocations made by the fixture are released.

Clang/GCC O0/O2, UBSan and MSVC x86/x64 match their corresponding originals.
All 54 staged and 54 production source/layout checks pass. All 48 project
configurations select C11 for both producers and preserve all 28 C/C++ basename
pairs. The eight changed sources match 216 historical versions across 27 earlier
conversion stages. Full D3D9 Debug and Release executables link without launching
the game. Diagnostics use static compiler runtimes and verify GCC imports.

Run `utils/tests/test_anim_clump.ps1` for quiet verification. Full isolated checks
use `events_project.py --anim-clump`, `check_events_project.py --anim-clump` and
`build_events_project.py --anim-clump` (plus `--release`). Diagnostics stay in
`.diagnostics/anim-clump-c-tests` and leave the user's `build/` directory untouched.
Regenerate normal project files for the clump `.cpp` to `.c` rename and the new
`AnimBlendList.c` producer.
