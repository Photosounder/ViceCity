# Player vehicle audio helper conversion

AudioVehiclePlayer.c owns UpdateGasPedalAudio and AddPlayerCarSample as C11 operations. Car and bike processing calls the gas operation directly. The remaining player-engine routine calls the C sample operation for cruising, revving, releasing the accelerator and idling. The two former member implementations and declarations are removed.

Gas smoothing reads the original gas pedal before selecting the vehicle type, applies fabsf, then mutates the original car or bike gas-audio field through existing address adapters. The 0.09 rise, 0.07 fall, original conditional minima/maxima and unsupported-type return are preserved. No allocation, copied gas-audio state or new game adapter is introduced.

Player samples preserve volume attenuation, PS2 bank selection, desktop bank zero, frequency, counter and priority, looping versus one-shot frame behavior, emitting volume, live loop-offset sample arguments, speed, maximum distance and feature-dependent reverb/reflection setup. Requests use the actual C request and ordered queue units.

Verification:

- Both complete function bodies, all internal callers, owner declarations, constants and conditional sound macros audited against saved originals
- All 40 earlier audio source audits pass alongside the new helper audit
- 620 consumer compile checks cover x64/x86, D3D9/software rendering, threaded/single audio and WAV/all decoder configurations
- 20 Clang/GCC O0/O2 before/after comparisons cover default, vanilla, no reverb, no external audio and PS2 shapes
- Four native MSVC x86/x64 before/after comparisons cover default and no reverb; actual game audio and physical consumers compile
- 65,536 operations per comparison cover alternating owners, repeated calls, car and bike smoothing, unsupported types, negative and signed-zero pedals, infinity and NaN smoothing inputs, rise/fall thresholds and overshoot, distance attenuation boundaries, byte-volume boundaries, looping values 0/1/255, arbitrary frequency/bank/counter arguments, valid and invalid sample indices, partial/full queue insertion and rejection, and callback changes between loop-start/end queries
- Clang undefined-behavior and float-cast-overflow sanitizer comparison passes

Snapshots and results are in build/audio-vehicle-player-c-tests. Run utils/tests/test_audio_vehicle_player.ps1 for the audit, comparisons and sanitizer fixture. Native and consumer compile scripts are in that build directory. Full game linking and in-game playback have not been verified.

The next major operation is ProcessPlayersVehicleEngine. It still owns shared LastAccel, CurrentPretendGear, traction/handbrake history, cruising and acceleration-channel history, plus a timer initialized on the first call. Its conversion must retain those shared lifetimes, player-entry reset, replay/exit controller selection, drivetrain traction checks, gear timing, cruising transitions and goto, acceleration-channel initialization failure, PS2 channel distinctions, camera translation, frequency time scaling and final vehicle gear-change mutation. The C sample helper now removes one of that routine's member-call dependencies.
