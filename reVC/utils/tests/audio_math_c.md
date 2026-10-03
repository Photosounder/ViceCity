# Scalar audio math C conversion

`src/audio/AudioMath.c` is a real C11 translation unit replacing six
AudioManager methods: volume attenuation, emitting-volume attenuation, stereo
pan, front/rear mix, Doppler frequency, and random displacement. All game callers
use the C exports; the old member declarations and definitions are removed.

Panning takes a scalar coordinate instead of a CVector pointer. Doppler takes
camera-switch status, elapsed time, and speed of sound explicitly. Random
displacement takes the existing five-entry table and retains its original shared
static direction and adjustment history, including zero-seed behavior and
unsigned wrapping. Owner construction and its five startup random draws remain
in their original order.

The original formulas, conversions, pan table, saturation, and early returns
are retained exactly. No additional guards or arithmetic simplifications are
introduced. Tests cover defined inputs; out-of-range floating-to-integer
conversions in the legacy algorithms are outside this comparison.

Run `utils/tests/test_audio_math.ps1` from the repository root. Its generator
compares the actual C statements, macros, and table against saved C++ methods,
and audits all edits to the four game caller files and owner header. Earlier
audio source audits account for these known call substitutions through
`audio_math_migration.py`; the math audit independently checks the actual kernel
bodies. Saved originals and native check scripts are in
`build/audio-math-c-tests`.

The executable fixture checks all byte volumes, attenuation thresholds and
neighboring floats, positive/negative pan coordinates, Doppler gates and speed
clamping, and 300,000 random-displacement calls across changing tables and
zero/large seeds. It links every export from C. Baseline/current results match
under Clang and GCC at O0/O2, and native MSVC x86/x64. All six runs produce digest
`10103914456804643557`.

All 250 consumer compile checks pass. Native MSVC compiles the actual C module
and major game integration files; object symbols confirm C linkage and retained
owner startup/destructor callbacks. Existing sound, position, reset, and
pedestrian regression matrices are checked separately. The Visual Studio project
and filters register the C11 source/header; CMake and premake already collect C
translation units.

This verifies compilation and controlled baseline comparisons. A full linked
game build and in-game audio playback remain untested.
