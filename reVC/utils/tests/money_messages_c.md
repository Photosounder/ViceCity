MoneyMessages.c replaces both CMoneyMessage and CMoneyMessages with a plain C
record, a private sixteen-record pool and four C operations. The complete
initializer, manager renderer, record renderer and registration algorithm are
transcribed from the original source. The C SpecialFX controller calls this C
manager directly; its two former C++ money-message forwarding functions are
removed. Object and pickup callers retain an explicit by-value CVector input
boundary before entering the same C registration algorithm.

Actual MSVC Win32/Win64 probes verify the original sixty-byte record aligned to
four: timestamp at zero, position at four, sixteen uint16_t text units at sixteen,
RGBA at forty-eight, size at fifty-two and opacity at fifty-six. Matching C11
size, alignment and offset assertions run in each native and portable comparison.
Fourteen typed backend operations expose the existing sprite, font, screen and
Unicode owners without assuming C++ or RenderWare object layouts.

Initialization still clears only timestamps. Registration still leaves alpha
and the unused text tail untouched, takes the first free slot, converts text
before reading the current timer, and captures its input position before the
converter can modify the pool. Timestamp zero retains its original inactive
meaning. Unsigned lifetime subtraction, exact two-second expiration, position
rise, scale arithmetic and clamp order, conditional FIX_BUGS screen scaling,
color narrowing, font state and live screen/record reads are preserved.

The standalone FONT_LOCALE expression retains its original Japanese query and
does not set the font style. Projection uses a real typed RwV3d local, copied to
the C result only on success. All vector additions, including additions of zero
to x and y, remain in C with the original floating-point behavior. The original
fixed and unfixed free-slot conditions are unchanged. The unfixed full-pool
condition reads beyond the original array, so undefined full-pool oracle calls
are excluded from unfixed comparisons; the fixed configuration exercises full
pool rejection with a null text input. Text inputs stay within the original
sixteen-unit capacity and alpha inputs stay in the original defined range.

money_messages_fixture.py audits eight complete changed source/project files,
all four algorithms, both class removals, exact C header and typed backend.
Immutable original methods run unchanged against the same owner providers as
the C11 implementation. Only private access is exposed in the oracle header;
declared fields and method bodies retain their original types and layout.
The actual original AsciiToUnicode loop, CVector operators and Draw_ScaleY body
participate in the comparison. Separate C callers exercise the actual C API,
and C++ callers exercise the real legacy value boundary.

Each default run compares 2,178 complete pool/font snapshots, 31,104 projection
attempts, 26,698 prints and 1,250 conversions across 2,000 mixed frames. Cases
cover every legal occupancy, partial initialization, exact expiry and timer wrap,
timestamp zero, empty/fifteen-byte/high-byte text, retained text tails and alpha,
both registration inputs, real pool-position capture, converter recursion,
projection rejection, scale clamps including nonfinite distance inputs, signed
zero, optional PC scaling and languages, screen changes between font operations,
record mutation after projection and nested initialization/manager rendering.
Every declared field is observed independently of padding and process addresses;
finite float bits compare exactly and NaNs compare by classification.

Twenty-four Clang/GCC O0/O2 comparisons cover default, non-librw, unfixed,
PC-scaling, disabled-language and FINAL profiles. Six additional comparisons
trap undefined behavior and float-cast overflow. Five optimized MSVC/LTCG
comparisons cover Win32/Win64 D3D9/GL3 and vanilla Win32. Thirty actual production
compilations cover all affected callers, both C units and the backend; pickup
compilation explicitly enables MONEY_MESSAGES to include the optional call site.
Debug and Release x64 D3D9/OpenAL projects compile and link with existing
dependencies and Git build events disabled. No game executable is launched.

All fifty project configurations retain unique object paths, now including the
twenty-first C/C++ pair, MoneyMessages.c.obj and MoneyMessages.cpp.obj. All 106
preceding whole-source conversion audits pass. Immutable originals, commands,
logs and runtime records are under build/money-messages-c-tests. The quiet checks
are exposed by test_money_messages.ps1; build/finalize_money_messages.py verifies
the recorded final evidence.
