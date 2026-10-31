# TexRead C conversion (applied)

Production now uses the complete C texture reader. The whole source baseline
captured after the applied C TxdStore migration is in `.diagnostics/tex-read-c-tests/before`. The
complete tested candidate remains in `overlay`. Do not regenerate source or fixtures while compilers
are using them. All checks stay outside the user's `build` directory, and the
game is never launched.

The conversion moves all 16 original function definitions across the LIBRW and
legacy branches into `TexRead.c`. This includes native texture reads and timing,
whole and two-stage dictionary reads, failure cleanup callbacks, GPU capability
file persistence, DXT checks, progress drawing, error/input loops, archive
conversion, sector padding, directory writing, and optional settings restoration.
All decisions, loops, calculations and mutation ordering remain in C.

The source has 13 unique function names. With GTA_PC active, 13 definitions are
active under LIBRW and 12 under the legacy SDK; the three duplicate definitions
are the backend alternatives for capability read/check/write. The original
two-stage `numberTextures` and `streamPosition` state is retained, including the
initial `-1` count and the original behavior on early failure.

The API uses opaque SDK pointers, the existing C `CRect` record and a flat
16-byte GPU capability record. Legacy reference output parameters become C
pointer parameters, and their callers explicitly pass addresses. Existing
constants move unchanged into shared C-compatible headers for streaming offsets,
CD sector size and OS-version values. Their original headers include these
shared declarations, preserving one definition rather than copying SDK values
into the algorithm.

`TexRead_native.cpp` contains 70 external primitives and one typed SDK callback
trampoline. These bind SDK operations, borrowed owner state and remaining C++
owners. Callback contexts are local to each synchronous enumeration so nested
enumerations retain their own visitor and data. Native texture output slots
remain correctly typed and copy to the C caller only on success. The SDK's
dictionary-writing result is `const`; its C boundary preserves that qualifier.
Progress coordinates and colors are calculated in C; native drawing leaves
construct only the SDK-facing values and perform one original draw operation.

## Checks completed

- 56 GCC/Clang strict C11 producer and standalone C/C++ API compilations pass on
  x86/x64. Producer profiles cover legacy declarations, LIBRW D3D9, GL3, classic
  OpenGL without the selected fixes/features, MASTER, and non-PC preprocessing
- Sixteen MSVC compilations pass against the actual SDK headers for the entire
  original C++ producer, the C candidate, its forwarding unit and its actual native leaf unit on
  x86/x64, under default and custom allocator preprocessing
- `tex_read_stage_audit.py` verifies whole source projections, all 16 original
  definitions, shared constant identity, absence of C++/SDK types in C policy,
  compiler evidence and exact production integration

The proprietary legacy SDK directory (`sdk/rwsdk/include`) is absent in this
workspace. Legacy C preprocessing and syntax pass; actual legacy SDK compilation
and linking remain unverified. No fake production SDK mappings are supplied.
The successful owner compilations with the custom allocator macro do not prove
the whole game's custom allocator works; its existing heap restrictions remain.

## Runtime and integration evidence

- Twenty complete original/C runtime comparisons pass using actual MSVC x86/x64,
  `/Od` and `/O2`, D3D9, classic OpenGL, GL3, legacy and MASTER preprocessing
- Each comparison executes 111 public operations and 105,614–105,906 external
  events, creates and destroys all 107 controlled SDK textures, exercises five
  typed cleanup callback entries, and releases all 18 real raw backing
  allocations. The classic profile creates 45 dictionaries; other profiles
  create 41. Runtime hashes vary by architecture/profile and, for some x86
  profiles, optimization; every corresponding original/C pair matches exactly
- Fixtures compile the entire original/C producer, entire generated native unit
  and independent public callers, plus the actual `Directory.c`, `General.c`
  and `Rect.c`. Original capability declarations are shared verbatim in the
  fixture without replacing any original function body
- Cases cover successful, failed and null native reads; live game-state changes;
  timing and repeated anisotropy queries; empty/odd/even dictionaries; independent
  resumed streams; failure cleanup in both phases; early failed first-stage
  state; absent/valid/changed/partially read/unwritable capability data; DXT
  short circuiting; progress-screen coordinates/colors; early frame failure;
  bounded escape and preexisting-quit error loops; archive success/open failures;
  serialization and directory failures; missing loaded dictionaries; exact sector
  padding; and cleanup/settings behavior on the original early-return paths
- The original three dictionary-store forwarding ABIs now live in
  `TexRead_bindings.c`. All three are linked and exercised in each runtime
  comparison. Their old C++ definitions are removed
- GPU capability record size, alignment and all offsets match between original
  C++, new C++ consumers and standalone C on x86/x64: 16 bytes, alignment 4,
  field offsets 0/4/8/12
- Fourteen complete affected producer/consumer units compile against the actual
  SDK across x86/x64 before and after application
- Both C units select C11 in all 48 evaluated configurations. All 28 existing
  paired object names remain distinct. Normal projects contain 425 source
  units; legacy Win32 RW34 configurations contain 424
- Full D3D9 Debug and Release builds link (14,343,168 and 4,955,136 bytes).
  Compiler tracking records prove `/TC /std:c11` for the two C units, and the
  linked object timestamps are checked
- 378 whole historical source projections pass before and after application.
  Preceding dictionary-store, heap, allocator, collision and animation audits
  remain required and pass through complete explicit source successors

Nine existing source paths change, including the producer rename and shared
constant includes. Seven files are added: the C API, native boundary header and
unit, C forwarding unit, and three shared constant headers. Existing native
callers use the C API with explicit SDK pointer conversions. `RwHelper.h`
includes the C declarations, removing the old SDK-typed helper signatures and
legacy C++ reference declaration.

The reader preserves backend-specific position signedness at its C boundary.
Controlled legacy fixtures use the unsigned SDK memory-position representation;
actual proprietary SDK compilation remains unavailable as noted above. Mocked
legacy runtime success is not a claim that the missing SDK can build or link.

The authoritative gate is
`tex_read_stage_audit.py --production --integration`. It requires every applied
candidate byte, strict and actual SDK compiler evidence, all runtime variants,
record layouts, full configuration/link checks, historical source proofs and
complete preceding-owner audits. The project-wide conversion continues with
remaining C++ owners.
