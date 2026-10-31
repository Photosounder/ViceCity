# Complete frame-name plugin conversion

The next owner after RpAnimBlend is src/rw/NodeName.cpp. Its eight functions own
registered frame-name storage, lifecycle, copying and stream callbacks. The only
SDK dependencies are plugin registration and raw stream I/O. The public functions
have five complete consumer files: the animation plugin native primitive unit,
vehicle and clump model information, file loading and main plugin attachment.

The staged conversion moves every algorithm and the private zero-initialized
32-bit registered offset into NodeName.c. NodeName.h is self-contained C with the
existing public names and an opaque frame handle. NodeName_backend.h declares
four SDK primitive leaves and the C callback ABI. NodeName_native.cpp contains
those four leaves and two typed stream callback trampolines. No name-storage,
copying, read-termination or registration-result algorithm remains native.

Preserved original details:

- Construction clears the first name byte only when the global registered offset
  is positive, irrespective of callback offset and size arguments.
- Destruction returns the object. Copy uses strncpy for exactly 23 bytes, leaves
  byte 24 untouched, and returns null. Truncation does not force a terminator.
- Stream read ignores the returned byte count, then terminates at binaryLength
  using the current global slot. Write returns the input stream even when the
  SDK write fails. Existing SDK length contracts and lack of bounds checks stay
  unchanged; invalid buffers are not invented as new diagnostic inputs.
- Size measures the current slot. Plugin attachment always registers streams
  after object registration, ignores stream registration failure, and treats only
  the exact object-registration result -1 as attachment failure. Public lookup
  returns null for any negative offset. Initial offset zero is preserved.
- The plugin ID remains MAKECHUNKID(rwVENDORID_ROCKSTAR,0xFE), 0x0253F2FE, with a
  native compile-time assertion and the original 24-byte plugin allocation.

Completed validation:

- Twelve Clang/GCC x86/x64 strict C11 producer and standalone C/C++ API compiles.
- Fourteen complete-source real MSVC compiles under x86/x64: the C owner, native
  primitive unit and all five complete consumer files, with USE_D3D9 enabled.
- Four whole-producer runtime comparisons (Clang/GCC O0/O2), one undefined
  behavior sanitizer comparison and two real MSVC x86/x64 comparisons pass.
  Each has 4096 cases and 1764866 checks, 3072 copies/reads/writes, 4608 object
  and stream registrations and 512 reentrant read-boundary registrations.
  Both complete original and C producers hash to bbf04eacfc552a4d.
- Fixtures capture actual SDK callback signatures. They cover -2/-1/zero/positive
  registration offsets, ignored callback offset/size arguments, exact copy
  padding and truncation, partial reads/writes, SDK write failure, and global
  slot changes during reentrant stream reads. They compare all initialized
  object bytes, not synthetic native record layouts or process pointer values.
- GCC diagnostics link static runtime libraries, audit imports for unwanted
  MSYS/libgcc/libstdc/libwinpthread DLLs, and run on a Windows-only runtime PATH.
  Error dialogs are disabled. Neither GUI tests nor game launches are used.
- All 48 evaluated project configurations select NodeName.c as C11 with no object
  collisions. Source count is 417, or 416 for legacy Win32 RW34; paired C/C++
  basenames remain 28. All 74 complete historical source projections pass.
- Full isolated D3D9 Debug and Release builds and links pass, producing
  executables of 14329856 and 4952576 bytes respectively. All
  checks stay under .diagnostics and do not recreate the user's build directory.

Production application is complete. Both edited files and both new primitive
boundary files exactly match the fully built candidate, with LF endings. All
fourteen real production source compilations and all 74 production historical
projections pass. The complete preceding plugin, clump, sequence, association
and frame-callback audits also pass against the explicit name-plugin successor.

The anim_plugin migration endpoint now wraps node_name. stats_successor resolves
the explicit NodeName.cpp -> NodeName.c rename and checks exact historical byte
digests before accepting a complete transformation. The archived animation
plugin project uses its private stage projection; its whole-source byte proofs
permit only explicitly verified successor conversions.

node_name_stage_audit.py --production --integration reports applied-verified.
All compiler handles are terminal. No git or GUI tests were used. The full
project conversion remains active; only this complete owner is finished here.
