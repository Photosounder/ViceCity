# Mission state and facade C conversion

AudioMissionState.c is an actual C11 translation unit implementing seven mission
state/policy functions and six public DMAudio mission facade functions. Eight C++
owner methods are removed, including the former coordinate-setter wrapper; the
public setter directly calls the existing AudioMissionPosition C kernel. Owner
records, label lookup, preloading, and spatial playback remain in their existing
modules. No owner fields or record layout change in this step.

The C functions preserve loading-status fallback, play-permission gates, ducking
exception, query results, and clear write order. Clear resets only its original
fields and stops stream slot+1 after those writes. Coordinates and unrelated load
failure history are retained. Initialized invalid-slot playing/finished queries
still return true. Each uninitialized query retains its own separate two-entry
uint32 counter array, starting at one and shared across all owners. Ducking still
consumes the playing-query counter through its original call.

Uninitialized invalid-slot queries and invalid-slot ducking retain their original
unsafe behavior; no additional validation is introduced. Those cases are excluded
from defined-input runtime comparisons. Other invalid-slot gates and no-op writes
are tested across all remaining byte slot values.

Run utils/tests/test_audio_mission_state.ps1. The generator independently audits
all seven C state bodies, all six facade bodies, complete edits to four caller
files, and the owner header. The fixture executes original member/facade bodies
and the actual C module with the original owner field declarations. Both versions
link the existing coordinate C kernel. A C caller exercises every public mission
export and live global access. The streamed-file stop is mocked and records the
state visible at callback time; no device is opened.

Tests cover initialized bytes 0/1/255, loading/play statuses 0/1/2/255, valid and
missing samples, the ducking exception, both slots, all initialized invalid byte
slots, coordinate updates, and retained phone/frame/coordinate state. Ten thousand
interleaved query steps across two owners test separate/shared counter history and
ducking calls. No claim is made that the runtime reaches uint32 counter overflow;
the original unsigned increment and modulus statements are separately audited.

All sixteen Clang/GCC O0/O2 comparisons pass across default, vanilla,
external-3D-disabled, and PS2-shaped builds. Native MSVC x86/x64 comparisons pass
with digest 343058897624947128. All 310 consumer compile checks and native game
integration compilation pass. Native symbols confirm thirteen C providers and
matching game calls. All relevant earlier source audits pass; eight earlier
position/reflection runtime comparisons also pass. Their geometry fixture retains
the saved thin setter solely to drive the unchanged coordinate kernel, while this
new fixture tests current facade integration.

The Visual Studio project/filter registers the C11 source; CMake/premake already
collect C files. Saved sources, native scripts, logs, and verification.json are in
build/audio-mission-state-c-tests. A full linked game build and in-game playback
have not been performed.
