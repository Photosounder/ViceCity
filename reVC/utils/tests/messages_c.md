The message manager now uses C11 for all 27 public operations, both complete
private comparison helpers and all four canonical tables. The CMessages static
class is removed. MessageQueues.c owns the brief queue, six big-message stacks,
previous-brief history and retained mission-title array. Messages.h exposes the
same three plain records and corresponding C APIs; all 25 full callers use them.

Messages.cpp contains eight typed native boundaries: original render-state setup,
locale query, ASCII-to-UTF-16 conversion, the full mutable controller action-name
array, command-key resolution, two HUD setters and the full native HUD clear.
Every queue scan, shift, partial reset, expiry decision, substitution, comparison,
local buffer and registration write is in C. The existing Timer/Pager C APIs are
called directly. The complete conditional controller-action enum and the original
40-unit action-name extent moved into ControllerActions.h; no values or controller
fields changed. The array reader returns a pointer to the exact native full array.

Before editing, native probes established tMessage/tBigMessage/tPreviousBrief sizes
of 44/176/32 bytes on x86 and 56/224/40 on x64. New probes compare original field
offsets/types, alignment, lifecycle traits, all 27 API signatures and all four
complete public table sizes. They also check the actual native action-array type,
whole controller size/alignment and action-array offset, including configurations
with BIND_VEHICLE_FIREWEAPON enabled and disabled. All new/touched files use LF.
The new C source has a unique basename and is registered as C11 in Visual Studio;
the existing CMake/Premake source globs include it.

Twenty portable comparisons cover Clang/GCC O0/O2 with default, classic languages,
MASTER, unfixed and GL profiles. Nine MSVC O2/LTCG comparisons cover default, GL,
MASTER and classic on x86/x64 plus Win32 vanilla. Five Clang O1 comparisons trap
undefined behavior and float-cast overflow. Every original/C pair matches. Each
run records 4,121 complete states, 1,236 big HUD calls, 206 brief HUD calls, 206
render-state entries, 197 ordered HUD/pager clears and seven mutating callbacks.
Default profiles record 26,141 pure locale reads, 1,309 numeric conversions and 31
key resolutions; classic profiles record zero locale reads, 1,315 conversions and
35 key resolutions. Record padding and native action-array extent are included:

    x64/default: c2ed635eaf1791af
    x64/classic: e2307165a9922c9a
    x86/default: e4ed8ed67ea5d2bf
    x86/classic: 26b4c4e497e1b59a
    x86/vanilla: fa06219da168815a

Vanilla has 44 action names; other tested runtime profiles have 45. Each run
performs over 43,000 independent assertions, including every byte of the original
canonical zero state. The oracle compiles all original public bodies, both raw
private helpers, original records and class-owned tables. After conversion it
compiles the actual production C unit and all eight native bindings. Every API
enters through a separate genuine C11 caller. Native comparisons use real project
common/types, config, timer/pager declarations and the shared controller enum.
Fixtures compile the entire original numeric Unicode converter and both HUD-copy
operations; input and graphics-state owners are observed without devices/windows.

Coverage includes partial Init/Clear and retained padding/title/history fields;
zero-size/null-source wide copies and untouched tails; 16-bit length wrap; prefix
and high-bit comparisons; null/empty numeric expansion and repeated sixth-number
substitution; initialized-tail string expansion; unknown/first-prefix key matching;
all twelve registration variants; full queue rejection with eager preprocessing;
head replacement versus second-slot insertion; all six independent big stacks;
strict expiry, unsigned deadline wrap and first-hole retention; native history
deduplication; duplicate small-print removal; the original early-stop big-print
search; HUD gating/style-five side effects; and ordered ClearAll delegation.
Callbacks mutate queues after render-state entry, later styles during an earlier
HUD call, brief content after style five, occupied slots/time during eager numeric
or key preprocessing, and live action names between placeholders. A sequenced
4,000-operation trace compares interleaved public APIs and all retained state.

Tests retain original buffer/index preconditions. Numeric values fit the original
ten-byte decimal temporary, HUD payloads fit their existing output arrays, and
public mutating string helpers receive initialized writable buffers. The original
InsertStringInString loop copies beyond the first source terminator up to its
computed total length. Standalone comparisons supply initialized trailing storage
to test that exact behavior. Registration/display comparisons use null or empty
replacement strings where their original local temporaries have defined contents;
nonnull replacement strings with uninitialized scratch tails are not claimed to
have portable defined behavior. Full original conditional source and fixed buffer
sizes are preserved rather than repairing those unrelated legacy contracts.

There are 135 production source compilations across x86/x64 D3D9/GL and Win32
vanilla, covering the actual adapter, every caller and the C controller. Actual
x64 D3D9/OpenAL Debug and Release builds compile and link. Historical Timer/Pager
audits additionally prove the complete old Messages.cpp producer relocation.
Pager's consumer adapter now calls the new C owner; its standalone comparison
retains the unchanged native dependency observations, with the full new C owner
tested separately here. Immutable originals, layout/type/compiler/build logs and
result records live in build/messages-c-tests. No graphical test or game was run.

Run utils/tests/test_messages.ps1 with optional -Sanitized, -Native, -Consumers,
-BuildProject and -AuditPrevious switches.
