# Date records and operations in C11

`CDate` is now a plain C record with the original six `int` fields. `Date.cpp` is removed and its five complete operations are implemented in `Date.c`: initialization, later-than, earlier-than, equality and field population. No constructors, operators or reference parameters remain in the date API.

The ten comparisons in `MemoryCard.cpp` call `Date_IsAfter` or `Date_IsBefore`. Local dates are initialized at their original declaration points, in declaration order. The compile-date member is explicitly initialized before the memory-card constructor body uses it. The intervening `CMemoryCardInfo` constructors do not access dates or their owner; their original bodies are checked as part of diagnosis. Static date arrays retain their zero-initialized fields. September/October parsing and all other surrounding menu/save statements are unchanged.

Population uses pointers for the five original signed-byte reference inputs and a signed 16-bit year value. It preserves sequential reads and assignments. Comparisons retain signed values and the original year/month/day/hour/minute/second priority, including invalid calendar values. The serialized fragment remains six four-byte fields in the original second/minute/hour/day/month/year order: 24 bytes.

Run the quiet comparisons from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_date.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_date.ps1 -Sanitized
```

The baseline compiles the complete original class, constructor and all operators. The converted model links the actual C11 kernel and a separately compiled C11 caller. Each run covers 2,097,152 record comparisons and 32,768 save/load fragments. Independent checks cover signed population, shared input addresses, read-only inputs, field priority, comparison symmetry/trichotomy, global zero initialization, 24-byte layout and exact guarded unaligned save bytes. Reset, arbitrary signed integers, assignment, self-comparisons and all five operations are exercised. Every run yields `860f1c88fba21222 comparisons=2097152 fragments=32768 bytes=24`.

Four Clang/GCC runs cover O0/O2, two native Win32/Win64 runs use `/O2 /GL /LTCG /OPT:REF`, and one Clang O1 run traps undefined behavior. Native checks compile both real save consumers with `START_FROM_LAST_SAVE` enabled and disabled plus the C unit: ten production source compilations. Two additional header probes compare the complete original/current sizes, alignments and field offsets of the date, memory-card owner and card-info types on Win32/Win64.

Both native forced `PS2_MENU` compilations still report exactly the same eight original errors for GTA III level identifiers and `CStreaming::HaveAllBigBuildingsLoaded`. The new date APIs, initialization sites and record type introduce no additional errors in that branch. This unsupported configuration is not reported as a passing build.

The full source audit checks all five method bodies, field types/order, all ten comparisons, construction points, save statements and every complete transformation of the eight existing files against immutable snapshots. All 88 preceding source audits also pass. Visual Studio explicitly compiles the renamed unit as C11; the existing CMake/Premake C globs include it. Tests do not open windows and do not establish full game linking or gameplay.

The later file-manager conversion exposed an eleventh comparison in `Frontend.cpp`
that this phase's two-consumer native check missed. The resume-last-save loop now
uses `Date_IsBefore(&SlotDate[saveSlot], &SlotDate[i])`. The broader native frontend
compile passes, and original/new complete selection loops match across 32,768 slot
histories on each architecture. See `filemgr_c.md` and `build/test_filemgr_date.py`.
