# LiftPlanner

Qt6/QML app built on the `libs` submodule (repo `app-libs`). Builds for Linux
(AppImage) and Android (APK/AAB).

## Layout

`src` is split into layers, and each layer into topics. Dependencies point only
downwards: `domain → infrastructure → application → ui`. One target per layer.

```
app.env                  every app parameter: name, target, version, Android package, AppImage metadata
src/main.cpp             entry point: engine, QmlRegistrator, singletons, automation server
src/liftplannerapplication.*  composition root, next to main.cpp
src/domain/              entities, value objects, queries, repository ports   (domain)
src/infrastructure/      SQLite repositories, serializers, storage wiring     (infrastructure)
src/application/         services returning Task<T>                          (application)
src/ui/models/           QML-facing models
src/ui/viewmodels/       view models
src/ui/presentation/     formatting and QML helpers                          (ui)
src/ui/qml/              QML module, URI LiftPlanner — Main.qml, Theme.qml, screens
tests/                   GoogleTest suite
libs/cpp                 async, utils, platform, qmlutils, dbtoolkit, eventbus, agent, automation
libs/qml                 theme (Themed.Theme), themed (Themed.Components), app (App.Components), icons
```

The topics are `workout`, `exercisecatalog` and `userprofile`, and each appears
in every layer that needs it. An entity, its repository port, its SQLite
implementation and its service therefore live in `domain/workout/`,
`infrastructure/workout/` and `application/workout/` — split by topic **inside**
a layer, never a topic directory holding its own layers.

`src` is the include root, so a header is addressed by layer and topic:
`#include "domain/workout/workout.h"`,
`#include "infrastructure/workout/workoutrepositorydb.h"`,
`#include "application/workout/workoutservice.h"`.

`libs/cpp` is the include root, so libs headers are reached by concern:
`#include "async/task.h"`, `#include "platform/haptics.h"`,
`#include "qmlutils/qmlregistrator.h"`, `#include <dbtoolkit/dbtoolkit.h>`. Libs
targets are named `app_<directory>` (`app_async`, `app_platform`, `app_qmlutils`,
`app_utils`, `app_dbtoolkit`, `app_theme_qml`, `app_themed_qml`,
`app_components_qml`, `app_icons`, …).

## Scratch space

Use `tmp/` in the repo root for everything throwaway: helper scripts, logs, gdb
output, copies of the database, offscreen run dirs. It is gitignored.

Do not use `/tmp` or the session scratchpad — `tmp/` keeps this project's
working files next to the code and survives across sessions.

## Never touch the real user data

Test and debug runs must not mutate:

- `liftplanner.db` in the repo root
- `~/.local/share/LiftPlanner/`

The UI session script does the sandboxing for you: the `sandbox` section of
[.ui_automation.json](.ui_automation.json) makes it run the app in `tmp/run`
with a copy of the database and its own `XDG_DATA_HOME`:

```sh
python3 libs/tools/ui_session.py start --platform offscreen
python3 libs/tools/ui_driver.py find ""
python3 libs/tools/ui_session.py stop
```

Session state and the app log live in `tmp/ui_session/`. Pass `--platform`
instead of exporting `QT_QPA_PLATFORM`, so the command stays a single simple
call. The automation server is compiled in when `LIBS_AUTOMATION` is on (the
default for Debug); the port comes from `APP_AUTOMATION_PORT`.

## Build

```sh
cmake -S . -B build-desktop -DCMAKE_BUILD_TYPE=Debug
cmake --build build-desktop -j$(nproc)
ctest --test-dir build-desktop

./libs/scripts/build-android.sh debug apk
./libs/scripts/build-appimage.sh
```

The root `CMakeLists.txt` is composed from the helpers in
`libs/cmake/AppProject.cmake`: `app_load_env`, `app_project_setup`,
`app_add_module`, `app_add_qml_module`, `app_add_test`, `app_configure_android`.
Every app parameter lives in `app.env`; CMake reads it through `app_load_env()`
and the scripts source the same file, so there is no second copy to keep in
sync. Never bump the version by hand — `build-android.sh … official` does it.

`ctest -j` is safe: `tests/integration/main.cpp` points `XDG_DATA_HOME` at a
per-process `QTemporaryDir`, so each test gets its own `current_workout.json`
cache.

## Database access

All DDL and DML goes through the dbtoolkit query builders. No raw SQL.

## Code style

No comments in code — write it so it explains itself. Run the `format` CMake
target (or the `format-code` skill) before committing. The style is
`libs/.clang-format`; the repo-root `.clang-format` is a symlink to it.

## QML

The helpers glob sources at configure time, so **a new `.qml` or `.cpp` file
needs a CMake reconfigure**, not just a rebuild:

```sh
cmake -S . -B build-desktop
```

`qmlcachegen` catches syntax and some type errors at build time, but not wrong
property names or bad bindings — those only show up at runtime.

Never `target_link_libraries` between QML modules — the library drops out of
`DT_NEEDED` and the type disappears at runtime even though the build passes.

## Theming

`src/ui/qml/Theme.qml` derives from `DefaultTheme` (shipped by `libs/qml/theme`,
URI `Themed.Theme`) and is registered as the `Theme` singleton of
`Themed.Components`. It overrides only the groups that differ from the default
plus the LiftPlanner-specific ones (`layout`, `chip`, `drum`, `week`, `stat`,
`timer`, `setRow`). **Do not import `Themed.Components` inside `Theme.qml`** —
Theme is registered into that module, so the import creates a cycle and every
start logs `Cyclic dependency detected`.

Overriding a group replaces it wholesale, so an override must carry every
property that `DefaultTheme` and the themed components read from it.

## Commits

`<Verb> <Subject>[: <details>]`, English, imperative, capitalized, no trailing
period. No Conventional Commits prefixes and **no attribution trailer**. Stage
explicitly (`git commit -- <paths>`); never `git add -A`. Do not push unless
asked.

Body is **one paragraph of at most five lines**, or nothing at all when the
subject already says it. No second paragraph, no bullet lists.

`libs` is a submodule: when it changed, commit inside `libs` first, then commit
the app together with the new submodule pointer.
