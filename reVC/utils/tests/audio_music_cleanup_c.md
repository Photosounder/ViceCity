Unused music members
====================

Removed ServiceAmbience, ChangeRadioChannel, Enable and Disable after checking
production references. The first, third and fourth bodies were empty; the
second only returned true. No production callers remained. The exact startup
constructor and every stored record field remain unchanged.

Ten Clang configuration checks, twenty native MSVC consumer compiles including
the game-render caller on x86/x64, and all 81 preceding source audits pass.
No new behavioral fixture is needed for removal of unused placeholders.
The music owner now has only its startup constructor as a C++ member; all
operational music code already uses C functions.
