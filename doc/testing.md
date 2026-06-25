# Testing

Tests use GoogleTest and are built only for non-Android configurations
(`enable_testing()` is gated on `NOT ANDROID` in the root
[CMakeLists.txt](../CMakeLists.txt)).

## Build & run

```bash
# Configure (once)
cmake -S . -B build-desktop -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build everything (app + tests)
cmake --build build-desktop

# Run the whole suite
ctest --test-dir build-desktop

# Verbose / single test
ctest --test-dir build-desktop -V
ctest --test-dir build-desktop -R WorkoutServiceTest
```

## Layout

The [tests/](../tests/) tree mirrors `src/` layout — one suite per layer.

| Path | Covers |
|------|--------|
| [tests/workout/domain/entities/](../tests/workout/domain/entities/) | `Workout`, `Exercise`, `Set` entity behavior |
| [tests/workout/infrastructure/serializers/](../tests/workout/infrastructure/serializers/) | entity ⇄ `QVariantMap` round-trips |
| [tests/workout/infrastructure/database/](../tests/workout/infrastructure/database/) | `WorkoutRepositoryDb` against SQLite |
| [tests/workout/application/](../tests/workout/application/) | `WorkoutService` use cases |
| [tests/userprofile/application/](../tests/userprofile/application/) | `UserProfileService` |
| [tests/integration/](../tests/integration/) | cross-layer flows via `TestApplication` |
| `tests/third_party/googletest/` | vendored GoogleTest (skipped by `format`) |

Services expose their internals to their unit test via `LIBS_TEST_FRIEND(...)` (see
[utils/testing.h](../src/utils/testing.h)), so a test can drive the private `…Core()`
methods directly.

## Integration tests — `TestApplication`

[tests/integration/testapplication.h](../tests/integration/testapplication.h) builds a
self-contained object graph against an in-memory / throwaway SQLite connection: its own
`BackendWorker`, storage, repositories, services, and the three workout viewmodels. A
test gets the real wiring without `main.cpp` or QML.

Key helpers:

- `activeWorkoutViewModel()`, `workoutHistoryViewModel()`, `plannedWorkoutViewModel()`,
  `workoutService()` — the wired collaborators.
- `drain()` — pump pending worker-thread/event-loop work so async `Task` continuations
  resolve before assertions (services run on a background thread).
- Time-travel: `timeProvider()`, `setCurrentDate()`, `setCurrentDateTime()`,
  `advanceDay()`, `advanceDays(n)`, `advanceDate(target)`.

Existing integration suites:
[workout_lifecycle_test.cpp](../tests/integration/workout_lifecycle_test.cpp),
[active_workout_test.cpp](../tests/integration/active_workout_test.cpp),
[planned_workout_test.cpp](../tests/integration/planned_workout_test.cpp),
[time_travel_test.cpp](../tests/integration/time_travel_test.cpp).

## Time-travel in tests

Runtime "now" is read through `TimeProvider` (never `QDateTime::currentDateTime()`
directly), so tests can control the clock deterministically. `TestApplication` installs
a [MockTimeProvider](../src/utils/mocktimeprovider.h); call `setCurrentDate(...)` to pin
the date or `advanceDays(n)` to move forward. This is what makes assertions on weekly
activity, planned-vs-history boundaries, and date-stamped records reproducible. See
[time_travel_test.cpp](../tests/integration/time_travel_test.cpp).

## UI automation (desktop)

Beyond unit/integration tests, the desktop build exposes a TCP automation server so the
app can be driven end-to-end by `objectName`. Use the `ui-session` skill, or the scripts
directly:

```bash
# Ensure the app is up (adopts an existing instance on port 49210, else launches it)
python3 tools/ui_automation/liftplanner_session.py start

# Drive it (one command per call; the instance persists between calls)
python3 tools/ui_automation/liftplanner_driver.py find ""        # list objectNames
python3 tools/ui_automation/liftplanner_driver.py dump           # tree + properties
python3 tools/ui_automation/liftplanner_driver.py click startWorkoutButton
python3 tools/ui_automation/liftplanner_driver.py set_time 2026-01-05
```

The driver mirrors the test clock control (`set_time`, `advance_time`, `reset_time`),
so UI smoke scenarios can be made deterministic the same way integration tests are.
Discover controls with `find` / `dump` rather than reading QML. The concrete scenarios
live in [doc/test-cases/](test-cases/).

## See also

- [doc/test-cases/](test-cases/) — UI smoke scenarios for the driver.
- [doc/architecture.md](architecture.md) — the layers these tests exercise.
