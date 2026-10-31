ModelIndexData.c owns all 136 dynamic model-ID globals, their minus-one static
initialization, all three registry operations and all fifteen classification
operations. ModelIndexData.h supplies the complete original MODELINDICES macro,
signed scalar declarations, fixed ModelIds.h enums and C operation contracts.
ModelIndices.h remains a compatible aggregate header: C sees only C declarations,
while C++ retains the original transitive ModelInfo.h dependency.

Dynamic IDs have C linkage and remain canonical mutable int16_t variables, with
distinct stable addresses. Native C++ consumers see the same two-byte signed
types. This allows subsequent C modules to keep actual pointers to model IDs
without shadow values, binding constructors or delayed initialization.

Name matching retains the exact original declaration order, case-sensitive
General_faststrcmp calls, first-match assignment and immediate return. There are
136 variables but only 133 distinct names: roadworkbarrier1, trafficlight1 and
lamppost1 each occur twice. The original first entry wins; later variables with
the same name are not assigned by that lookup. Reset still assigns minus one to
every variable, and TestModelIndices remains empty.

C owns all classification branches and short-circuit order. Four typed native
readers retain model lookup/downcast, the existing non-const building predicate,
and the captured model's code-glass and artist-glass flags. IsGlass keeps the
model pointer captured by its first lookup. Native model layouts and methods
remain unchanged. No new null guard, model-ID clamp or lookup validation was
introduced.

The fixture audits four complete modified files and all three new files against
immutable originals and the explicit migration. It holds native model info,
fixed ID enums, General and common sources unchanged. Every original global,
registry operation and complete inline helper body compiles in the C++ oracle.
Both programs link actual General.c, with only its comparison symbol renamed so
an observing wrapper can execute that unchanged real implementation. RNG stubs
abort if unused General operations unexpectedly consume or change random state.

The corpus compares all canonical variables and distinct addresses, every name,
matching order and all 65,536 signed-ID values for the fourteen numerical
classifiers. It covers duplicate names, case changes, suffixes, unknown names,
all ID setters, repeated resets, alias values and empty debug calls. Hooks change
the borrowed input string after a comparison and mutate a target variable before
its final assignment. The typed model simulator exercises all glass/building
combinations, a real nonzero derived-to-base offset, mapping replacement after
pointer capture, and changed glass flags after the building predicate. Each run
observes 46,471 comparisons and 2,004,484 classification calls.

Twenty Clang/GCC O0/O2 comparisons cover default, plain, MASTER, unfixed and GL
profiles. Five Clang runs enable undefined-behavior and float-conversion traps.
Seven optimized native MSVC comparisons cover Win32 default/GL/MASTER/vanilla and
Win64 default/GL/MASTER. Four fail-fast stubs cover unused interpreter dependencies
from the actual native common headers; any call aborts. Native probes verify all
136 variable types, eighteen operation signatures, native model access/flag
types, downcast relationships and a standalone C11 public header.

Fifty production compilations cover nine representative native consumers and
the C unit across five architecture/feature combinations. Actual Debug and
Release x64 D3D9/OpenAL projects compile and link. Fifty project configurations
retain unique object paths for all twenty-eight paired C/C++ units. All 127
preceding source audits pass. Historical model-header audits continue comparing
the complete relocated macro/declarations and all fifteen complete helper bodies;
they also check the compatibility aggregate. No game or GUI test was launched.

Run utils/tests/test_model_indices.ps1. Optional switches select Sanitized,
Native, Consumers, BuildProject and AuditPrevious. Immutable originals, generated
inputs, logs and manifests are in build/model-indices-c-tests.
