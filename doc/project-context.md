# LiftPlanner — LLM Context

Concise, source-derived context for working on this repo. For the generic Clean-Architecture
*template/blueprint* see [architecture-for-llm.md](architecture-for-llm.md) — but **this file is
authoritative** for what actually exists; the blueprint describes patterns not all implemented here.

## What it is
Qt6 C++/QML workout-planning desktop+Android app. Polish-language UI. Version `1.0-rc2`,
Android package `com.liftplanner.app`. SQLite DB file `liftplanner.db`.

**Core idea:** the app does *not* contain training AI. Instead it generates a structured GPT prompt
(profile + history + planned workouts), the user pastes it into an LLM/ChatGPT, gets back a JSON plan,
and imports it via clipboard. The whole "intelligence" lives in
[src/data/gpt_prompt_template.txt](../src/data/gpt_prompt_template.txt) (interview + planning state
machine, progression rules, FINAL OUTPUT SCHEMA). Read that file before touching prompt/JSON logic.

## Architecture (Clean Arch / DDD, 4 layers)
`UI → Application(Services) → Domain ← Infrastructure`. Domain has no DB/UI deps.

**Reality vs blueprint:** there is **no** `Result<T>`, **no** `UserContext`, **no** `EventBus`.
Services take a plain repository reference and return plain types (`std::vector`, `std::optional`,
`int`, `bool`). Entity IDs are plain `int` autoincrement (`m_id{-1}`), **not** UUID-BLOB.

## Modules (`src/modules/<name>/{domain,application,infrastructure}`)
### workout
- **Entities** ([domain/entities](../src/modules/workout/domain/entities)):
  - `Workout` — id, name, createdTime, plannedTime, startedTime, endedTime, `WorkoutStatus`
    {Planned, Started, Ended}, `vector<Exercise>`. Methods `start()`, `end()`, `isCompleted()`.
  - `Exercise` — id, workoutId, name, description, restSeconds (default 120), youtubeLink,
    `vector<Set>`. `setsToString()`.
  - `Set` — id, exerciseId, repetitions(int), weight(double, supports .5), completed(bool).
- **Repository**: `WorkoutRepository` interface (findAll/findOne/save/remove/count/exists, all take
  `WorkoutQuery`). `WorkoutQuery` = fluent optional filters (whereStatus, wherePlannedTimeIsNotNull,
  orderByPlannedTime, withLimit…).
- **Service** `WorkoutService`: loadPlannedWorkouts, importPlannedWorkouts, removeAllPlannedWorkouts,
  loadHistory, importHistory, findWorkout, saveWorkout, deleteWorkout.
- **Infrastructure**: `WorkoutRepositoryDb` owns the 3 nested tables (`workouts`, `exercises`, `sets`)
  + serializers. Separate `ExerciseRepositoryDb`, `SetRepositoryDb`. Tables created in
  `WorkoutRepositoryDb::createTables()`. Column-name constants live in each serializer
  (e.g. `WorkoutSerializer::table="workouts"`, keys id/name/created_time/planned_time/started_time/
  ended_time/status).

### userprofile
- `UserProfile` entity: userId, language(="en"), timezone(="UTC"), `Sex`, sessionsPerWeek(=3),
  `ExperienceLevel`, `PrimaryGoal`, `optional<QDate> dateOfBirth`, `optional<double> bodyweightKg`,
  `UnitSystem`{Metric,Imperial}, notes. Single-row profile.
- `UserProfileService`: load()/save()/exists(). Feeds profile + notes into the GPT prompt.

## Core
- [src/liftplannerapplication.cpp](../src/liftplannerapplication.cpp) — composition root. Owns
  `AppDbStorage`, the two services, and the ViewModels. `initialize()` wires everything;
  `registerQmlTypes()` exposes singletons to QML.
- `AppDbStorage` ([src/core/storage](../src/core/storage)) — opens SQLite, `PRAGMA foreign_keys=ON`,
  creates tables, hands out `workoutRepo()` / `userProfileRepo()`.
- [src/core/types.h](../src/core/types.h) — shared enums (`SortDirection`).

## UI layer
- **ViewModels** (`src/ui/viewmodels`), registered as QML singletons under module `LiftPlanner` with
  **renamed** instance names:
  - `ActiveWorkoutViewModel` → QML `ActiveWorkoutService`: startWorkout, completeCurrentSet,
    navigateToNext/Previous, endWorkout, duplicateSet, removeSet.
  - `WorkoutHistoryViewModel` → QML `WorkoutHistoryService`: loadAllWorkouts, save/deleteWorkout,
    importFromJson/Clipboard, exportToClipboard, recentWorkoutsToJson, `lastWorkout`.
  - `PlannedWorkoutViewModel` → QML `PlannedWorkoutService`: loadAll, importFromClipboard,
    importFromJson, **generatePrompt** (fills the template with date/profile/history/planned),
    `workouts`, `nextWorkout`.
  - `ClipboardHelper` → QML `ClipboardHelper`.
- **Models** (`src/ui/models`): `WorkoutModel`/`ExerciseModel`/`SetModel` — QObject wrappers exposing
  entities to QML via Q_PROPERTY; `toEntity()`/`clone()`. Property boilerplate via `DECLARE_PROPERTY`
  macro in [src/utils/serializationutils.h](../src/utils/serializationutils.h).
- **QML** (`src/ui/*.qml`): `Main.qml` entry, `MainView.qml` + `BottomNavigation`, screens
  `ScreenHome`/`ScreenPlannedWorkouts`/`ScreenActiveWorkout`/`ScreenHistory`, dialogs `RestDialog`,
  `ValueEditorDialog`, `NotificationPopup`. Module URI `LiftPlanner`, root `qrc:/LiftPlanner/ui`.
  `Theme.qml` is a QML singleton (also re-exported into `Themed.Components`).

## libs/ (git submodule — git@github.com:dpietruchowski/app-libs.git, shared across projects)
- **dbtoolkit** (SHARED lib): `DbStorage` (QSqlDatabase wrapper), `DbRepository` (generic CRUD on
  QVariantMap), `query/` fluent SQL builder (Where/Order/Select/Insert/Update/Delete/CreateTable/
  Column/Join/Alias), `modelrepository.h`.
- **utils** (static): `QmlRegistrator` (registerSingletonInstance/registerType/registerEnums/
  registerSingletonType, `QML_LIVE_ENABLED` path swaps to context properties + live reload),
  `TimeProvider`+`MockTimeProvider`, `ColoredSvgProvider`, `UrlHelper`, `WordMatcher`,
  `FillInTextBuilder`, `testsupport.h`.
- **qmlcomponents** (QML module URI `Themed.Components`): ThemedButton/Card/Dialog/Input/ComboBox/
  CheckBox/Switch/SpinBox/TextArea/ProgressBar/ProgressBorder/Toast/Icon/Page.
- **qmllive**: dev-only `FileWatcher` for QML hot reload.
- **icons**: shared SVG resources (app-specific icons are in [src/ui/icons](../src/ui/icons)).

## JSON import/export
[src/utils/workoutjson.h](../src/utils/workoutjson.h) namespace `WorkoutJson`: full JSON (file/clip)
+ **compact** JSON (GPT prompt) + parser that accepts both. Schema matches the prompt's
FINAL OUTPUT SCHEMA (`user_profile` + `workouts[].exercises[].sets` as `"5x60kg,5x75kg,..."` string).

## Build & test
- **Desktop:** standard CMake. Qt6 (Core/Gui/Qml/Quick/Sql), C++20, CMake 3.16+.
  ```bash
  cmake -S . -B build/Debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  cmake --build build/Debug --parallel
  ctest --output-on-failure --test-dir build/Debug
  ```
- **Android:** [build.sh](../build.sh) `[clear] [debug|release]` — builds in Docker
  (`stateoftheartio/qt6:6.8-android-aqt`) into `build-android/`. Signing config + keystore in root
  CMake / `sign.sh` / `generate_key.sh`.
- Tests = GoogleTest/GMock submodule (`tests/third_party/googletest`). Layout mirrors modules:
  `tests/workout/{domain,application,infrastructure}`, `tests/userprofile/...`, plus
  `tests/integration/` (full-app lifecycle tests via `TestApplication`). Tests excluded on Android.
- After clone: `git submodule update --init --recursive`.

## Conventions
`PascalCase final` classes, `m_` members, `camelCase` methods, `enum class`, `#pragma once`,
no comments (self-documenting), member-init lists, file name == class name (lowercase). Strict
warnings `-Wall -Wextra -Wpedantic -Werror`.

## Status / open work
See [.claude/TODO.md](../.claude/TODO.md) (functional TODO + Google Play release blockers). Current
top functional gaps: no manual workout/exercise editing (AI-import only), no user-profile editing UI,
no DB schema versioning, previous-session set values, Android button sizing.
