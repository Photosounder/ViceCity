# Bone mapping conversion

This stage replaces the complete Bones.cpp producer with genuine C11 and exposes
its two existing free functions through C linkage. All 22 bone tag constants,
17 supported ped node mappings and literal bone names retain their original
values and spelling. Unknown bone tags still return null. Unsupported ped nodes
still invoke the project re3_assert handler in non-MASTER builds and return -1
when that handler returns. MASTER builds retain the original fallback result.

The original PedNode enum moves verbatim out of PedModelInfo.h into PedNode.h.
The native model header includes the shared C header; none of the model class's
fields or methods change. Bones.c now includes only C-compatible headers and has
no SDK/native algorithm adapters. The existing caller names require no call-site
rewrites; the C linkage is supplied by the public Bones.h declarations.

Evidence under .diagnostics/bones-c-tests:

- Eight Clang/GCC O0/O2 original/C comparisons, default and MASTER profiles,
  exercise 4101 input values, including the complete -2048..2048 range and four
  signed integer boundary values. All 41 enum constants are also compared.
- Original non-MASTER behavior: 24577 checks, 4084 returning project assertions,
  hash 0c0682c36e8e1281. MASTER: 12325 checks, zero reports, f73567a2526e1b1a.
- Two matching UBSan runs and four matching actual MSVC x86/x64 runtime runs.
  Calls cross an independently compiled strict C11 caller in both phases.
- Fourteen native compilations cover the actual complete producer and all six
  native callers, with coherent real SDK/project headers in both pointer widths.
- Native COFF layout values prove that CPedModelInfo stays 72 bytes on x86 and
  96 on x64, with every inspected original field offset and both enum widths
  unchanged. PedNode values are copied from the original declaration, not rebuilt.
- 48 evaluated build configurations retain unique object names and the C11
  producer; 102 exact historical projections cover three changed paths across
  34 earlier conversion stages.

All artifacts use .diagnostics, not the user's disposable build folder. Test
binaries link statically, reject GCC/MSYS runtime DLL imports, suppress native
error dialogs, and run with only Windows system paths. Full game builds are
linked but never launched. Before/overlay trees and fixtures must not be
regenerated while a compiler using them is alive.

Production application is complete. finalize_bones.py --production reports
applied-verified: all 14 production compilations, four production native runtime
comparisons, both model layouts and all 102 historical projections pass. Full
D3D9 Debug/Release links pass with 14318592/4949504 byte executables. The manager
finalizer also passes through the explicit bone-stage successor projection. The overall main-project C conversion remains active after
this helper stage; native model classes and other animation owners still remain.
