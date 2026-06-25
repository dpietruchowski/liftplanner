# Modules

Feature code lives under [src/modules/](../src/modules/), one directory per bounded
context. Each module is split into `domain` (entities, repository interfaces, queries),
`application` (services), and `infrastructure` (serializers, DB repositories). There are
two modules today: **workout** and **userprofile**.

## workout

[src/modules/workout/](../src/modules/workout/)

### Domain

- [Workout](../src/modules/workout/domain/entities/workout.h) — aggregate root. Holds
  `id`, `name`, the timestamps `createdTime` / `plannedTime` / `startedTime` /
  `endedTime`, a [WorkoutStatus](../src/modules/workout/domain/entities/workoutstatus.h)
  (`Planned` / `Active` / `Ended`), and a `std::vector<Exercise>`. Exposes behavior
  (`start()`, `end()`, `isCompleted()`, `addExercise()`, `totalSets()`,
  `totalRepetitions()`) and `validate()`.
- [Exercise](../src/modules/workout/domain/entities/exercise.h) — `name`,
  `description`, `restSeconds`, and a `std::vector<Set>`.
- [Set](../src/modules/workout/domain/entities/set.h) — `repetitions`, `weight`,
  `completed`.
- [workoutquery](../src/modules/workout/domain/repositories/workoutquery.h) and
  [WorkoutRepository](../src/modules/workout/domain/repositories/workoutrepository.h) —
  the repository interface and query objects the service depends on (the service never
  touches SQL directly).

### Application — [WorkoutService](../src/modules/workout/application/workoutservice.h)

Async use-case methods, each returning `Task<T>` and delegating to a private
`…Core()` that returns `Result<T>` run on the worker thread:

- `loadPlannedWorkouts()`, `importPlannedWorkouts()`, `removeAllPlannedWorkouts()`
- `loadHistory(limit)`, `importHistory()`
- `topExercises(topN, recentWorkouts)` → `ExerciseFrequency { name, count, bestOneRepMax }`
- `findWorkout(id)`, `saveWorkout(workout)`, `deleteWorkout(id)`

"Planned" vs "history" is a status distinction on the same `workouts` table, not a
separate table.

### Infrastructure

- Serializers map entity ⇄ `QVariantMap` and own the column-name constants:
  [WorkoutSerializer](../src/modules/workout/infrastructure/serializers/workoutserializer.h),
  [ExerciseSerializer](../src/modules/workout/infrastructure/serializers/exerciseserializer.h),
  [SetSerializer](../src/modules/workout/infrastructure/serializers/setserializer.h).
- [WorkoutRepositoryDb](../src/modules/workout/infrastructure/database/workoutrepositorydb.cpp)
  composes the exercise and set repositories, creates all three tables
  (`createTables()`), and registers migrations. Exercises and sets are owned via
  `ON DELETE CASCADE` foreign keys.

### ViewModels (UI bridge)

- [ActiveWorkoutViewModel](../src/ui/viewmodels/activeworkoutviewmodel.h) — drives an
  in-progress session: `startWorkout`, `completeCurrentSet`, `navigateToNext/Previous`,
  `duplicateSet`, `removeSet`, `endWorkout`; emits `workoutCompleted`.
- [WorkoutHistoryViewModel](../src/ui/viewmodels/workouthistoryviewmodel.h) — history
  list, `lastWorkout`, `topExercises`, `weekActivity`, JSON/clipboard import-export. It
  observes `ActiveWorkoutViewModel` via Qt signals to keep weekly activity and history
  in sync.
- [PlannedWorkoutViewModel](../src/ui/viewmodels/plannedworkoutviewmodel.h) — planned
  workouts; depends on both `WorkoutService` and `UserProfileService` (it feeds the
  profile into the AI prompt and imports the returned JSON plan).

## userprofile

[src/modules/userprofile/](../src/modules/userprofile/)

### Domain

[UserProfile](../src/modules/userprofile/domain/entities/userprofile.h) plus the value
enums [Sex](../src/modules/userprofile/domain/entities/sex.h),
[ExperienceLevel](../src/modules/userprofile/domain/entities/experiencelevel.h),
[PrimaryGoal](../src/modules/userprofile/domain/entities/primarygoal.h),
[UnitSystem](../src/modules/userprofile/domain/entities/unitsystem.h). The profile
carries language, timezone, sex, sessions-per-week, experience level, primary goal,
date of birth, bodyweight, unit system, and free-text notes. It is a single-row
profile (`user_id` primary key).

### Application — [UserProfileService](../src/modules/userprofile/application/userprofileservice.h)

`load()`, `save(profile)`, `exists()` — same `Task<T>` / `…Core()` `Result<T>` pattern
as `WorkoutService`.

### Infrastructure

[UserProfileSerializer](../src/modules/userprofile/infrastructure/serializers/userprofileserializer.h)
and
[UserProfileRepositoryDb](../src/modules/userprofile/infrastructure/database/userprofilerepositorydb.cpp)
(single `user_profile` table; most columns `NOT NULL` with defaults).

### ViewModel

[UserProfileViewModel](../src/ui/viewmodels/userprofileviewmodel.h) exposes the profile
to QML ([ScreenProfile.qml](../src/ui/profile/ScreenProfile.qml)).

## AI-assisted planning

The planning flow does not call an LLM from inside the app. The profile + request is
turned into a prompt from [src/data/gpt_prompt_template.txt](../src/data/gpt_prompt_template.txt),
copied to the clipboard, and the user pastes the model's JSON answer back; the JSON is
parsed by [workoutjson](../src/utils/workoutjson.h) and imported through
`WorkoutService::importPlannedWorkouts`.

## See also

- [doc/architecture.md](architecture.md) — layers and data flow.
- [doc/data-model.md](data-model.md) — the concrete SQLite schema.
