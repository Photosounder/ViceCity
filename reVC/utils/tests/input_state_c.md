# C input-state records

`InputState.h` replaces the controller, keyboard, mouse and mouse-pointer helper
classes with C records. `InputState.c` owns all ten operations and the canonical
mouse inversion state. The existing pad manager and platform callers use the C
API; six native primitives in `Pad.cpp` retain the device/frontend interface.

The conversion preserves the original partial operations:

- Controller activity ignores `NetworkTalk`; controller clear resets all channels.
- Stick normalization divides by `32767.0f`, including the negative endpoint below
  minus one.
- Keyboard clear leaves `ENTER` unchanged.
- Mouse initialization resets buttons and movement, leaving the original pad byte
  outside its initialization contract. Mouse clear resets only buttons and keeps
  both movement representations and the existing pad byte.
- D3D capability selection retains its exact one-through-eight button cases,
  three-axis wheel condition, repeated availability checks, short-circuit menu
  exclusivity and original capability-query failure contract.
- GLFW selection retains the cursor X default, zero comparison and resulting
  button/wheel availability. The helper's inversion fields remain unused by this
  operation, as before.

The three static mouse records invoke C initialization at their former constructor
positions. The tests include literal original/projected global declarations with
a preceding raw-storage write, so initial zero-filled storage cannot mask a lost
initialization call. The inversion record retains its original static zero state.
The other records remain trivial to initialize, copy and destroy. Mouse records
become trivial records with explicit initialization; no destructor work existed.

Each platform profile compares 69,100 states using the literal original methods,
independent C11 callers and the literal native primitive bodies. Checks exhaust
all signed stick values, activity channels, keyboard byte patterns and mouse
button combinations. They cover movement signed zero, infinities, NaNs and
subnormals, menu/device creation cases, capability boundaries, cursor defaults
and observable device-call order. Clang/GCC O0/O2, undefined-behavior checks and
native MSVC x86/x64 agree. Capability-query tests use defined successful results;
the conversion retains the original uninitialized-on-failure behavior.

Real native headers verify all 79 record field types/offsets and the 276-byte
enclosing pad layout. Source compiles cover the pad, frontend, controller config,
recording and Windows skeleton, plus the existing D3D-guarded GLFW translation
unit. The C producer and isolated device policies are also checked in GLFW mode.
These checks do not launch a GUI or require a running input device.

Checks live under `.diagnostics/input-state-c-tests`. All 48 generated
configurations compile the new producer as C11 and retain distinct object names
for the 28 existing C/C++ source pairs. Both full D3D9 configurations compile and
link without running the game. Earlier conversion audits still pass.

Run `utils/tests/test_input_state.ps1` for quiet behavior and native-source checks.
Use `events_project.py --input-state`, `check_events_project.py --input-state` and
`build_events_project.py --input-state` (plus `--release`) for the isolated full
project.
