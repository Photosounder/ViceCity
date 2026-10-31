The manually extended Visual Studio project assigned both Clock.c and Clock.cpp
to Clock.obj, and likewise for FileMgr, IniFile, Timer, ZoneCull, Zones, Draw,
Lines and WindModifiers. The C object could overwrite the C++ owner-reader
object, removing the symbols required at link time. Existing isolated comparisons
used separate object paths, so they did not expose this project-level collision.

All eighteen paired source entries now have explicit ObjectFileName metadata,
including the source extension: Clock.c.obj and Clock.cpp.obj, for example.
Other source object names are unchanged. The migration endpoint preserves this
metadata when auditing earlier immutable source/project snapshots and handles
future paired source entries through the same complete project transformation.

object_names_fixture.py evaluates the actual project using MSBuild -getItem in
every configuration and verifies that every enabled source produces a unique
object path, including all nine pairs. The checked-in Premake generator already
disambiguates matching basenames; a separate generated C/C++ Pair probe confirms
it assigns Pair.obj and Pair1.obj, so no generator source change is necessary.

The actual Debug and Release x64 D3D9/OpenAL reVC builds compile and link
successfully. Build events are disabled in the verification commands to avoid
the unrelated Git hash command; existing library dependencies are reused.
The Debug test output is kept under build/object-collision-tests/debug-output.
No game executable was launched. Logs and command/result records are in
build/object-collision-tests. Native build subprocesses use a deduplicated
Windows environment to avoid duplicate PATH/Path keys in MSBuild tool tasks.

The subsequent reference-management conversion adds a tenth paired compilation
unit, References.c/References.cpp, with the same explicit per-source object names.
The all-configuration collision regression now checks all ten pairs.

The antenna conversion adds Antennas.c/Antennas.cpp as the eleventh pair, with
the same distinct per-source object metadata and all-configuration checks.

The rope conversion adds Ropes.c/Ropes.cpp as the twelfth pair and extends the
same distinct-object regression across every project configuration.

The skidmark conversion adds Skidmarks.c/Skidmarks.cpp as the thirteenth pair,
using the same per-source names and complete configuration regression.

The rubbish conversion adds Rubbish.c/Rubbish.cpp as the fourteenth pair, with
the same explicit per-source object names in all project configurations.

The water-cannon conversion adds WaterCannon.c/WaterCannon.cpp as the fifteenth
pair, retaining separate C and C++ object names in every project configuration.

The point-light conversion adds PointLights.c/PointLights.cpp as the sixteenth
pair and retains the same distinct-object checks in all fifty configurations.

The shared-buffer conversion adds RenderBuffer.c/RenderBuffer.cpp as the
seventeenth pair, with the same separate per-source object paths.

The motion-blur streak conversion adds MotionBlurStreaks.c/MotionBlurStreaks.cpp
as the eighteenth pair; all fifty configurations retain unique object paths.

The bullet-trace conversion adds BulletTraces.c/BulletTraces.cpp as the
nineteenth pair, retaining the same distinct-object check in all configurations.

The special-effects controller adds SpecialFX.c/SpecialFX.cpp as the twentieth
pair, retaining distinct source-extension-bearing object names in every configuration.

The money-message conversion adds MoneyMessages.c/MoneyMessages.cpp as the
twenty-first pair, preserving the same separate objects in every configuration.

The shiny-text conversion adds ShinyTexts.c/ShinyTexts.cpp as the twenty-second
pair with unique source-extension-bearing object names in all configurations.

The bright-light conversion adds BrightLights.c/BrightLights.cpp as the
twenty-third pair, keeping unique per-source objects in all configurations.

The boat-foam conversion adds BoatFoam.c/BoatFoam.cpp as the twenty-fourth
pair, preserving distinct C and C++ objects in all project configurations.

The 3D-marker controller adds Markers3D.c/Markers3D.cpp as the twenty-fifth
pair, retaining unique source-extension-bearing object names in every configuration.

The credits conversion adds Credits.c/Credits.cpp as the twenty-sixth pair,
retaining distinct objects for the complete C sequence and its typed owner calls.

The console conversion adds Console.c/Console.cpp as the twenty-seventh pair,
retaining unique objects for the C record/algorithms and typed font boundary.

The cloud effects conversion adds Clouds.c/Clouds.cpp as the twenty-eighth pair,
retaining distinct objects for C effects and the remaining typed sky-background code.
