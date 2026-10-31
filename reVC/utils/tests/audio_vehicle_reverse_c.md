# Vehicle reverse-gear sound conversion

AudioVehicleReverse.c owns the former ProcessReverseGear member as C11. Vehicle processing calls the explicit manager/parameter operation. Reverse eligibility, distance caching, modifier/volume/frequency calculations, airborne gas-audio decay and request setup run in C. Eight narrow adapters in AudioLogic.cpp read the original game fields or expose the existing gas-audio float address without changing game object storage.

The conversion retains the initial vehicle capture for model/engine/gas/gear checks, the later automobile capture for drive-wheel/audio state and the final live parameter vehicle read for sample selection. It preserves the caddy exclusion, reverse gas/gear short circuit, grounded transmission branch, airborne prior-contact decay, original ABS macro behavior including signed zero, conditional sample/index offsets, request fields and true return values for every in-range path. No initialization/pause/type/null gates are added. Private constants move into the C unit and the existing shared ModelIds.h supplies MI_CADDY. Visual Studio registers the C source; CMake and premake already discover C sources.

Verification:

- Complete generator body, all field/address adapters, private constants, ABS/conditional sound macros, internal caller and owner/header edits audited against saved originals
- All 38 current and earlier audio source audits pass
- 20 Clang/GCC O0/O2 before/after comparisons across default, vanilla, no reverb, no external audio and PS2 configurations
- Four native MSVC x86/x64 comparisons across default/no-reverb, plus compilation of actual game audio and physical callers
- 16,512 operations per comparison exercise both owners, actual C callers, repeated decay/request generation, range/model/engine/gear/gas gates, all three vehicle-read roles, cached distance flags, grounded/airborne states, prior contacts, reverse sample selection and empty/partial/full request queues
- Explicit airborne cases with null transmission pointers prove that branch leaves transmission access unused
- Actual C audio math, requested sound insertion and ordered sound queue units are linked in both implementations
- Complete owner, parameter and controlled vehicle/transmission bytes are observed with normalized pointer identities, plus query order and return values
- Model queries change the live parameter vehicle after the initial capture; drive-wheel and transmission queries change it after the automobile capture; backend start-offset queries change the sample index before the end-offset read
- 590 unique consumer compilation checks pass across architecture, D3D9/software, threading and decoder configurations
- An additional Clang O1 before/after comparison with undefined-behavior and floating-conversion checks in trap mode passes

Full game linking and playback were not tested. Game queries are controlled in the differential fixture. Accepted modifiers and emitting-volume conversions are bounded to the original defined numeric range; zero reverse velocity limits or accepted NaN modifiers are not introduced by the fixture.
