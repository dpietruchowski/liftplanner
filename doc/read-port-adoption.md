# Adopting the read port

The toolkit now has a second door next to `DbRepository`: `DbProjection<Row>`
answers **one SELECT → one typed row**, with the shape (`FROM`, `JOIN`, `WHERE`,
`ORDER BY`, `LIMIT`) and the projection (which columns come back and how they are
read) decided separately. See `libs/doc/read-port.md`.

This document says where LiftPlanner should use it, in what order, and what stays
on the aggregate repositories. Written before the code, so the end state is a
decision rather than an accident.

## What is wrong today

Every list in the app is served by an aggregate repository, so reading anything
means materialising everything underneath it.

| Caller | Reads | Displays |
|---|---|---|
| `WorkoutHistoryViewModel::loadAllWorkouts` | every ended workout with all exercises and all sets | name, date, per-exercise set summary |
| `WorkoutService::topExercisesCore` | the last 20 full workouts | two tiles: name, count, best 1RM |
| `WorkoutService::recentTotalsCore` | the last 20 full workouts, again | two tiles: total time, total distance |
| `WorkoutTemplateService::summarize` | every template with prescriptions, **then one catalog query per template exercise** | name, exercise names, set count |
| `ExerciseCatalogViewModel` | definitions with every muscle involvement | name, kind, equipment, primary muscles |

Three specific faults:

1. **`summarize` is a genuine N+1.** `CatalogDefinitionLookup::findDefinition`
   runs `findOne` per template exercise, and each of those loads the definition's
   muscle rows it never looks at. Ten templates of six exercises is sixty-one
   queries plus sixty muscle queries.
2. **Statistics are computed in C++ over aggregates.** `topExercises` and
   `recentTotals` load two full histories to produce four numbers. The database
   can group and sum them in one statement.
3. **Ordering happens after the limit in `topExercises`.** `loadHistoryCore(20)`
   cuts to the twenty most recent workouts and only then ranks exercises. That is
   the intended window, so it stays — but it is exactly the shape the read port
   warns about, and it must be stated rather than left implicit.

And one that is not about reading at all: **`saveWorkout` rewrites the whole
aggregate.** Completing a single set deletes every exercise row of the workout
and reinserts all of them with their sets. `ActiveWorkoutViewModel` calls it on
every completion, adjustment, reorder and set toggle.

## The end state

### Ports

Each topic gets a read port next to its aggregate port, in the same
`domain/<topic>/` directory:

```
domain/workout/workoutrowrepository.h          WorkoutRow, ExerciseFrequencyRow, TrainingTotalsRow
domain/workout/workouttemplaterowrepository.h  WorkoutTemplateRow
domain/exercisecatalog/exercisedefinitionrowrepository.h  ExerciseDefinitionRow
```

A row is a flat struct of already-computed fields. It has no identity, no
behaviour and no invariants — it is a snapshot, and nothing may mutate through
it. The ports declare only reads: `findAll`, `findFirst`, `count`, `exists`,
`findIds`.

### What stays on the aggregate repository

Writing, and fetching **one** aggregate to mutate it. `Workout`,
`WorkoutTemplate` and `ExerciseDefinition` remain the unit of mutation, because
those operations recompute state and check invariants afterwards. What leaves is
every list operation whose receiver only displays or filters.

We are deliberately **not** splitting `WorkoutRepositoryDb` into separate read
and write classes. Splitting the interface does not split the responsibility, and
the class stays one either way. The seam that matters runs along the tables:
a repository that writes an aggregate should not know another table's columns,
it should delegate to that table's repository.

### Single-field writes

Alongside the aggregate `save`, the workout port gets the operations the active
workout actually performs, each one `UPDATE`:

```cpp
bool setSetCompleted(int setId, bool completed);
bool setSetValues(int setId, const SetValues& values);
```

## Order of work

Each step is a commit, with the tests green.

1. **Query conditions through one builder.** `WorkoutRepositoryDb::buildWhereClause`
   still hand-rolls `where.isEmpty() ? x : where.and_(x)` at every branch, while
   the template and catalog repositories already use `addClause` from
   `infrastructure/whereclause.h`. Make them one shape. No behaviour change.
2. **`WorkoutTemplateRow` and its projection.** One `SELECT` joining
   `workout_templates → template_exercises → exercise_definitions` with a
   `COUNT` of prescriptions, replacing `summarize` and the per-exercise lookup.
   The biggest single win, and it removes `CatalogDefinitionLookup` from the
   list path.
3. **`ExerciseFrequencyRow` and `TrainingTotalsRow`.** `topExercises` and
   `recentTotals` become two `SELECT`s with `GROUP BY` over the windowed history,
   the window expressed as a subquery on workout ids so the limit is applied to
   workouts, not to joined rows.
4. **`WorkoutRow` for the history and planned lists.** Name, times, status,
   exercise count, set count, completed set count — enough for the cards. The
   active workout keeps reading the aggregate, because it mutates it.
5. **`ExerciseDefinitionRow` for the picker.** Muscle involvements are joined and
   aggregated rather than loaded as child rows.
6. **`count` / `exists` / `findIds` on a one-column projection**, replacing the
   places that count by materialising a list.
7. **Single-field set updates**, and `ActiveWorkoutViewModel` calling them
   instead of `saveWorkout` on every keystroke.
8. **List operations off the aggregate repositories**, once nothing calls them.
9. **A port contract test**, one suite run against the implementation, on a real
   database through a fixture. It must cover ordering combined with a limit and
   nullable columns — that is where a read port and an aggregate repository drift
   apart first. The gmock doubles in `tests/application` stop standing in for the
   repository's query building, which they never checked anyway.

## What was done

| Step | Outcome |
|---|---|
| 1 | done — `WorkoutTemplateConditions` is the one condition builder for templates, `addClause` and `isNullClause` for workouts |
| 2 | done — `WorkoutTemplateRow` from one SELECT over three tables; `CatalogDefinitionLookup` is off the list path |
| 3 | done differently — see below |
| 4, 5 | **not done on purpose** — see below |
| 6 | not needed — `DbRepository::count` and `exists` already issue `SELECT COUNT`; nothing counted by materialising a list |
| 7 | done — `WorkoutRepository::saveSet`, used by the two adjustment paths |
| 8 | done for templates — `WorkoutTemplateRepository` no longer lists |
| 9 | done — `WorkoutTemplateRowRepositoryDbTest` runs the port against a real database, covering ordering with a limit, an offset and a null column |

**Step 3 came out differently on purpose.** Grouping and summing in SQL would have
written the one-rep-max formula a second time, in a place where nobody would
notice it drifting from `Set::oneRepMax`. Instead the window is read as one flat
row per set in a single SELECT, and the formulas — now named in
`domain/workout/strengthmath.h` — are applied to those rows. The aggregate
materialisation is gone, the rule is still written once.

**Step 7 needed a prerequisite that was itself the bigger fix.** `saveChildren`
deleted every exercise of the workout and reinserted it, so a set's id changed on
every save and nothing could be addressed by id. Children are now updated in
place. A set the model has never persisted still has id `-1` and falls back to
the aggregate save; making the model learn its ids after a save would remove that
last case, and is the obvious next step.

## What was deliberately not done

- **Rows for the history and planned lists (step 4).** The cards bind to
  `workout.exercises` and each exercise's `sets` — the list genuinely displays
  the aggregate. A row would have to be followed by a second read per expanded
  card, trading one problem for another.

- **A row for the exercise picker (step 5).** The catalog already loads in two
  queries, not N+1: definitions, then all muscle involvements in one `IN`. The
  join would save one round trip and cost a `GROUP_CONCAT` that has to agree with
  how the domain orders primary muscles.

- **Splitting the repository implementations into read and write classes.**
  Splitting the interface does not split the responsibility. The seam that
  matters runs along the tables.

## Rules adopted

- **A row never travels back into a write.** If a caller needs to change
  something it read as a row, it loads the aggregate by id first.
- **Order and filter on the expression, never on the output alias.** SQLite
  accepts `calc_total` in `ORDER BY` but not in `WHERE`.
- **A limit without an order is a bug**, not a preference: it returns an
  arbitrary subset, and sorting that subset in C++ afterwards sorts the wrong
  set. Every shape that limits also orders.
- **An aliased join needs a natural key on the far side**, or rows multiply and
  `LIMIT` silently returns fewer entities than rows.
- **The same rule is written once.** Where a computed column duplicates a rule
  the domain already enforces in C++ — a workout's completed-set count, an
  exercise's one-rep max — the SQL is the projection of the domain rule and the
  domain keeps the definition. Any change goes to both, and the contract test is
  what catches it when it does not.
