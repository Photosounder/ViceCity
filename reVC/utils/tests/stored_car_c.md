# C stored cars

`StoredCar.c` owns all six former stored-car operations: Init, Clear, HasCar,
Assign, StoreCar and RestoreCar. `CStoredCar` is a plain 40-byte record with two
three-float values, two 32-bit scalars and six signed bytes. Its two trailing
padding bytes remain outside the original initialization and assignment contract.
The canonical 48-element safe-house array remains in the garage manager, with
the same 1,920-byte extent. Native x86/x64 checks retain the original enclosing
garage sizes, member offsets, copy construction and destruction behavior.

Init and Clear only write the model sentinel. Assign copies members in the old
order and returns the destination identity. Both stored-record save/load
assignments explicitly call it. The two noncompatible whole-garage assignments
also use an explicit member helper, including the C stored-record assignment;
this preserves nested padding behavior after removing the custom C++ operator.
Compatible garage serialization and the surrounding save cursor/size logic are
unchanged.

StoreCar retains all field reads, proof-mask construction, signed byte narrowing
and the original automobile cast for the car/bike bomb read. Non-car/bike records
retain their previous bomb byte. RestoreCar retains the dependency request before
readiness, the fixed component-count gate, boat-first selection, pool allocation
assertions, original constructors, bike standing state and ordered transform and
identity writes. Only car bombs are restored; fixed builds assign the player as
rigger when the stored bomb byte is nonzero. Ownership/unlock behavior is
unchanged, and proof restoration only sets selected flags without clearing any
flags supplied by construction.

The 49 native leaves are typed property reads/writes, existing engine operations,
three existing constructors and assertion reporting. The stored-car policy and
serialization decisions live in C. The remaining garage and vehicle owners stay
native until their own conversions.

Independent strict C11 callers compare the exact original methods with the C
implementation, using the literal new native leaves against the same host model.
Each run compares 28,928 states and 80,384 explicit invariants. Coverage includes
partial writes, destination padding, self-assignment, all proof masks, vehicle
kinds, component gates, model readiness, byte narrowing, quiet NaN/infinity/signed
zero payloads, native operation traces, constructor defaults, by-value vector
transfers, 40-byte save strides/guards and nested garage copies. Debug child
processes cover missing model info and allocation assertions for all three
construction branches without opening a dialog.

Clang/GCC O0/O2, UBSan and native MSVC x86/x64 agree in fixed/classic and MASTER
profiles. Native source checks compile both compatible and noncompatible save
branches. All 48 generated configurations require C11 for the new source and
retain distinct object names for the 28 existing C/C++ pairs. Full D3D9
Debug/Release builds run without launching the game.

Run `utils/tests/test_stored_car.ps1` for quiet checks. Full isolated project
checks use `events_project.py --stored-car`, `check_events_project.py --stored-car`
and `build_events_project.py --stored-car` (plus `--release`). Diagnostics stay
under `.diagnostics/stored-car-c-tests`. Regenerate project files to include
`StoredCar.c`, `StoredCar.h` and `StoredCar_backend.h`.
