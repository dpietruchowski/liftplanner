# Architecture

lift-planner is a Qt6/QML workout planner (C++20, SQLite via QtSql, CMake + Ninja).
It builds for Linux desktop (`build-desktop/src/liftplanner`) and Android (APK). The
code follows a layered Clean-Architecture / DDD split, with a UI-automation server on
desktop and shared infrastructure pulled from the `libs` submodule.

## Layers

Dependencies point downward only — an upper layer may use the one below it, never the
reverse.

```
┌──────────────────────────────────────────────────────────────┐
│ UI (QML)            src/ui/*.qml                              │
│   screens, components, Theme                                  │
├──────────────────────────────────────────────────────────────┤
│ ViewModels + Models src/ui/viewmodels, src/ui/models         │
│   QObject bridges exposed to QML as singletons               │
├──────────────────────────────────────────────────────────────┤
│ Application         src/modules/*/application                │
│   Services: async use-case methods returning Task<T>         │
├──────────────────────────────────────────────────────────────┤
│ Domain              src/modules/*/domain                     │
│   Entities (Workout, Exercise, Set, UserProfile), queries    │
├──────────────────────────────────────────────────────────────┤
│ Infrastructure      src/modules/*/infrastructure             │
│   Serializers + DB repositories (built on libs/dbtoolkit)    │
├──────────────────────────────────────────────────────────────┤
│ Core + libs         src/core, src/utils, libs/*              │
│   storage, app wiring, TimeProvider, BackendWorker, Task     │
└──────────────────────────────────────────────────────────────┘
```

## Composition root

[src/liftplannerapplication.cpp](../src/liftplannerapplication.cpp) is the single
place where the object graph is built and wired:

1. Constructs the [BackendWorker](../src/utils/backendworker.h) (a `QObject` owning its
   own `QThread`) and the [AppDbStorage](../src/core/storage/appdbstorage.cpp).
2. In `initialize()`, opens the database **on the worker thread** (via
   `QMetaObject::invokeMethod(..., Qt::BlockingQueuedConnection)`) and creates the
   services there, so all repository/SQLite access happens off the UI thread.
3. Creates the viewmodels on the UI thread, injecting the services.
4. `registerQmlTypes()` exposes the viewmodels to QML as singleton instances
   (`ActiveWorkoutViewModel`, `WorkoutHistoryViewModel`, `PlannedWorkoutViewModel`,
   `UserProfileViewModel`, `ClipboardHelper`) plus the `Notification` enums and the
   `ColoredSvgProvider`.

[src/main.cpp](../src/main.cpp) sets up `QGuiApplication`, the
`QQmlApplicationEngine`, the automation server (desktop), and loads
[src/ui/Main.qml](../src/ui/Main.qml).

## Data flow

A typical interaction runs top to bottom and back:

```
QML control  →  ViewModel (Q_INVOKABLE)  →  Service.method() : Task<T>
                                                   │ invoke(work) posts to worker thread
                                                   ▼
                                          …Core() : Result<T>  →  Repository  →  SQLite
                                                   │ Task resolves back on the UI thread
                                                   ▼
              ViewModel .then(this, …) updates models  →  emits *Changed  →  QML rebinds
```

- **Services are asynchronous.** Each public method (e.g.
  [WorkoutService::loadHistory](../src/modules/workout/application/workoutservice.cpp))
  wraps a private `…Core()` that returns `Result<T>` and calls `invoke(...)` from the
  [Service](../src/utils/service.h) base. `invoke` runs the work on the
  `BackendWorker` thread and returns a `Task<T>`; the caller attaches a continuation
  with `.then(this, [](T result){ … })` which runs back on the UI thread. See
  [WorkoutHistoryViewModel::loadAllWorkouts](../src/ui/viewmodels/workouthistoryviewmodel.cpp).
- **ViewModels expose state to QML** through `Q_PROPERTY` (often via the
  `DECLARE_PROPERTY` macro in [serializationutils.h](../src/utils/serializationutils.h))
  and notify changes with Qt signals. QML bindings react to the `*Changed` signals.
- **Cross-viewmodel coordination uses Qt signals/slots, not an event bus.** For
  example `WorkoutHistoryViewModel` connects to `ActiveWorkoutViewModel`'s
  `currentWorkoutChanged` / `workoutCompleted` to refresh weekly activity and history
  ([workouthistoryviewmodel.cpp:21-27](../src/ui/viewmodels/workouthistoryviewmodel.cpp#L21-L27)).

## Time

All runtime "now" goes through [TimeProvider](../src/utils/timeprovider.h)
(`TimeProvider::instance().currentDate()/currentDateTime()`) rather than
`QDateTime::currentDateTime()` directly. This makes time deterministic: tests and the
UI-automation driver can install a `MockTimeProvider` to pin or advance the clock. See
[doc/testing.md](testing.md).

## UI automation (desktop only)

On desktop the app starts a TCP automation server keyed off
`LIFTPLANNER_AUTOMATION_PORT`. An external Python driver
([tools/ui_automation/](../tools/ui_automation/)) finds controls by `objectName` and
issues `dump` / `find` / `click` / `set` / `invoke` / `set_time` commands against the
live instance. This is the basis for the smoke scenarios in
[doc/test-cases/](test-cases/) and the `ui-session` skill.

## Role of `libs`

[libs/](../libs/) is a shared git submodule (`app-libs`) used by both lift-planner and
its sibling app. lift-planner consumes:

- `dbtoolkit` — `DbStorage`, `DbRepository`, the query builder (`CreateTable`,
  `Column`, `Where`, `Order`) and `MigrationRunner` used by the infrastructure layer.
- `utils` — `Task` / `Result`, the `Service` base, `BackendWorker`, `TimeProvider` /
  `MockTimeProvider`, the QML registrator, and SVG provider helpers.
- `icons`, plus QML component libraries used by the UI.
- `eventbus` is present in `libs` but intentionally **not used** here — viewmodels
  coordinate through Qt signals/slots, which already provide the needed decoupling at
  this app's scale.

Because `libs` is shared, changes there are committed inside the submodule first, then
the pointer is bumped in the root repo (see the `commit` skill / `committer` agent).

## See also

- [doc/modules.md](modules.md) — the `workout` and `userprofile` modules in detail.
- [doc/data-model.md](data-model.md) — SQLite schema and relationships.
- [doc/testing.md](testing.md) — building, running tests, time-travel, UI automation.
