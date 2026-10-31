Both antenna classes are removed. Antennas.c owns the private main array, optional
reflection queue, all six former CAntennas algorithms and the complete former
CAntenna::Update spring algorithm. Antennas.h is a standalone C11 interface with
a plain three-float vector and 156-byte Antenna record. Antennas.cpp retains only
thirteen typed vector/RenderWare/debug-boundary operations and the original
private two-vertex RenderWare buffer. Five external consumer files call the C API;
the two vector callers copy through an actual typed CVector reader and preserve
the original pointer-ID narrowing to uint32_t.

Actual MSVC Win32/Win64 probes verify record size 156, alignment 4, bool offsets
0/1, ID offset 4, length offset 8, positions offset 12 and speeds offset 84. The
original CVector is 12 bytes/alignment 4. The actual Sqrt/RecipSqrt wrappers return
float, and the native probe object references sqrtf. The new normalizer retains
the original squared-sum order, 1.0f/sqrtf reciprocal, sequential x/y/z scaling,
and x-only fallback for nonpositive or NaN squared length.

The conversion preserves partial Init writes, updated-flag expiry, first-ID and
first-free selection, capacity rejection, sequential spring updates, ignored
replacement lengths for existing IDs, and both reflection duplicate checks. It
preserves every render-state call and vertex update in the C algorithms. The
reflection renderer still consumes its queue after failed transforms, and the
main renderer still disables vertex alpha even for an empty pool. Original debug
group labels and FINAL behavior are preserved.

antennas_fixture.py audits complete immutable old source bodies and all nine
changed source/project files. The differential comparison independently links
the entire old C++ implementation and actual new C11 implementation. Original
CVector operators, Normalise body and maths wrappers come from immutable source
snapshots. D3D and GL vertex records, setters and render-state enums come from
actual vendor/fakerw declarations. Only rendering sinks are mocked; native
comparisons compile the actual C++ common/main/vector/RW headers. Unrelated inline
interpreter dependencies abort if reached and are discarded by LTCG/OPT:REF.

The fixture observes all fields of every main/auxiliary record, inactive payloads,
live queue counts, exact render-call order and actual vertex fields, including
UV storage modified by previous transforms. It covers duplicate/full pools,
12,000 mixed operations, 4,096 direct spring updates, zeros, subnormals, infinities,
NaNs, pointer-ID narrowing, all/none/mixed transform outcomes, auxiliary queue
reuse, and reentrant rendering with the shared vertex buffer. All finite result
bits are compared; NaNs are compared by classification. Input generation has a
fixed evaluation order across compilers.

Twenty Clang/GCC O0/O2 original/C comparisons pass across five main/reflection,
D3D/GL and debug/final profiles. Five undefined-behavior trap comparisons pass.
Nine optimized MSVC/LTCG comparisons pass on Win32/Win64, including actual vanilla
Win32. Main profiles observe 1,150 full states and 20,620 transform attempts;
reflection profiles observe 1,163 full states and 23,365 attempts. All compilers
produce the same fingerprints for equivalent profiles.

Thirty-five actual production compilation checks cover the C11 unit, typed C++
backend and all five consumer files in D3D9 and software GL3 profiles on Win32 and
Win64, plus vanilla Win32. Actual Debug and Release x64 D3D9/OpenAL project builds
compile and link with existing dependencies; the Git hash events are disabled
and outputs stay inside build/antennas-c-tests. No game is launched. Distinct
Antennas.c.obj/Antennas.cpp.obj outputs extend the all-50-configuration collision
regression to eleven C/C++ pairs. Commands, results, logs and immutable snapshots
are under build/antennas-c-tests; test_antennas.ps1 exposes the main checks.
