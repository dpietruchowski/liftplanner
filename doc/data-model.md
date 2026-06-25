# Data model

The app persists to a single SQLite database opened by
[AppDbStorage](../src/core/storage/appdbstorage.cpp) (connection name
`liftplanner_connection`, file in the platform `AppDataLocation`). On open it runs
`PRAGMA foreign_keys = ON`, creates the tables if missing, and runs registered
migrations via `MigrationRunner`. Tables are defined with the `libs/dbtoolkit` query
builder (`CreateTable` / `Column`), not raw SQL.

## Tables

### `workouts`

The root of a workout. "Planned" workouts and "history" are the same table,
distinguished by `status`.

| Column | Type | Constraints |
|--------|------|-------------|
| `id` | INTEGER | PRIMARY KEY, AUTOINCREMENT, NOT NULL |
| `name` | TEXT | |
| `created_time` | TEXT | ISO-8601 datetime |
| `planned_time` | TEXT | ISO-8601 datetime |
| `started_time` | TEXT | ISO-8601 datetime |
| `ended_time` | TEXT | ISO-8601 datetime |
| `status` | TEXT | `Planned` / `Active` / `Ended` |

Source:
[workoutrepositorydb.cpp](../src/modules/workout/infrastructure/database/workoutrepositorydb.cpp).

### `exercises`

| Column | Type | Constraints |
|--------|------|-------------|
| `id` | INTEGER | PRIMARY KEY, AUTOINCREMENT, NOT NULL |
| `workout_id` | INTEGER | NOT NULL, FK → `workouts(id)` ON DELETE CASCADE |
| `name` | TEXT | |
| `description` | TEXT | |
| `rest_seconds` | INTEGER | |

Source:
[exerciserepositorydb.cpp](../src/modules/workout/infrastructure/database/exerciserepositorydb.cpp).

### `sets`

| Column | Type | Constraints |
|--------|------|-------------|
| `id` | INTEGER | PRIMARY KEY, AUTOINCREMENT, NOT NULL |
| `exercise_id` | INTEGER | NOT NULL, FK → `exercises(id)` ON DELETE CASCADE |
| `repetitions` | INTEGER | |
| `weight` | REAL | |
| `completed` | INTEGER | DEFAULT 0 (boolean 0/1) |

Source:
[setrepositorydb.cpp](../src/modules/workout/infrastructure/database/setrepositorydb.cpp).

### `user_profile`

A single-row table (one profile per install).

| Column | Type | Constraints |
|--------|------|-------------|
| `user_id` | INTEGER | PRIMARY KEY, NOT NULL |
| `language` | TEXT | NOT NULL, DEFAULT `'en'` |
| `timezone` | TEXT | NOT NULL, DEFAULT `'UTC'` |
| `sex` | TEXT | NOT NULL, DEFAULT `'other'` |
| `sessions_per_week` | INTEGER | NOT NULL, DEFAULT `3` |
| `experience_level` | TEXT | NOT NULL, DEFAULT `'beginner'` |
| `primary_goal` | TEXT | NOT NULL, DEFAULT `'general_fitness'` |
| `date_of_birth` | TEXT | ISO-8601 date |
| `bodyweight_kg` | REAL | |
| `unit_system` | TEXT | NOT NULL, DEFAULT `'metric'` |
| `notes` | TEXT | NOT NULL, DEFAULT `''` |

Source:
[userprofilerepositorydb.cpp](../src/modules/userprofile/infrastructure/database/userprofilerepositorydb.cpp).

## Relationships

```
workouts (1) ──< exercises (N) ──< sets (N)
   id            workout_id          exercise_id
                 ON DELETE CASCADE   ON DELETE CASCADE

user_profile  (standalone, single row)
```

Deleting a workout cascades to its exercises and their sets, so the workout aggregate
is removed as a unit.

## Migrations

Two mechanisms coexist:

- **`MigrationRunner`** (`libs/dbtoolkit`) — versioned migrations registered by each
  repository in `registerMigrations()` and run after table creation in
  [AppDbStorage::open](../src/core/storage/appdbstorage.cpp). Example: the exercises
  repo's `v1` migration drops the removed `youtube_link` column.
- **Ad-hoc `PRAGMA table_info` checks** — older databases are patched in place where
  needed, e.g. the `workouts.status` column is added and back-filled
  (`Planned`/`Ended`) for databases created before the status field existed
  ([workoutrepositorydb.cpp:51-71](../src/modules/workout/infrastructure/database/workoutrepositorydb.cpp#L51-L71)).

> Note: there is no global `PRAGMA user_version` migration framing yet — see the
> release notes in [.claude/TODO.md](../.claude/TODO.md).

## See also

- [doc/modules.md](modules.md) — entities and services that own these tables.
- [doc/architecture.md](architecture.md) — how persistence is reached from the UI.
