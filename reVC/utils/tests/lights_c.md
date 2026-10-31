The complete lighting owner is converted and verified. Evidence is recorded under `.diagnostics/lights-c-tests/verification-summary.json`.
All 19 original operations live in `Lights.c`, including both former ambient-colour overloads with separate C names, sun-vector cross products and normalization, brightness and weather branches, and extra-light selection policy.

`Lights_native.cpp` contains only 20 SDK primitives or external metadata reads. SDK handles are opaque C pointers; C++ SDK consumers cast those pointers at their actual use sites. Matrix-direction writes borrow scalar subobjects instead of aliasing incompatible vector records. SDK matrix and colour conversion uses byte copies with actual SDK size, alignment, and every field-offset assertion.

The original sun matrix writes only nine basis floats. Other matrix fields remain unspecified, as in the original owner. Runtime observations deliberately exclude those fields and the original local colours' unspecified alpha; librw's actual colour setter consumes only RGB. Extra-light strengths retain their original integer storage and float-to-integer truncation. No rendering-policy fixes are mixed into this conversion.

Run the prepare, strict C, complete native owner/client, fixture/runtime, layout, history, project generation/configuration, and Debug/Release link checks before `apply_lights.py`. After application, rerun production owner/client/history and full predecessor audits, then the integration stage audit. No test launches the game, and all generated files stay outside the independently cleaned `build/` folder.
