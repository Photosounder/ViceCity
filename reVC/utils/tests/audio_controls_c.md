# Audio backend controls C conversion

`AudioControls.c` is an actual C11 translation unit. Five DMAudio volume setters
now live there directly and call the existing SampleManager C interface, retaining
their original volume clamping. Ten provider/device operations take initialization
status explicitly where the old method gated backend access. Seventeen
AudioManager member declarations and definitions are removed, including the
initialization getter and acoustic-modeling setter whose callers now access the
same stored byte fields directly.

The provider-switch operation stays with the owner because it updates queue state.
Construction, destruction, startup random draws, queue fields, and backend lifetime
are unchanged. OpenAL provider-name clamping retains the original macro expression
and repeated backend queries; other backends retain the original rejection test.
This also retains the zero-provider edge case, uint8 provider-count truncation,
signed provider sentinels, and calls that deliberately bypass initialization gates.

Run `utils/tests/test_audio_controls.ps1`. The fixture generator audits the actual
C bodies against saved owner/facade statements, including the real Clamp macro,
and verifies all owner, header, facade, and host-adapter edits. It extracts the
original C++ implementations for the baseline and links the actual C source for
the converted implementation. Mock backend functions record ordering, arguments,
manager identity, and return values; this is a control-flow test, not audio playback.

The runtime matrix covers Clang/GCC O0/O2 with OpenAL, Miles, external-3D disabled,
and reflections disabled. It checks every volume byte, every provider ID byte,
initialization bytes 0/1/2/255, provider counts 0/1/3/255/256/513, signed sentinel
values, handle operations, CD operations, and acoustic-modeling state. All sixteen
comparisons pass. Native MSVC x86/x64 OpenAL baseline comparisons also pass, both
with digest `6767419034517411620`.

All 260 consumer compile checks pass. Native MSVC compiles the real C module,
AudioManager, AudioLogic, and DMAudio. Native symbols show fifteen C exports and
matching facade references. Earlier math, queue-reset, pedestrian, sound, position,
and entity source audits pass after accounting for these intended member removals.
No prior kernel implementation changes in this step.

CMake/premake already collect C files; the existing Visual Studio project/filter
registers the new source as C11 and its header. Saved sources, native scripts,
compile logs, and verification.json are in `build/audio-controls-c-tests`.
A full linked game build and in-game playback have not been performed.
