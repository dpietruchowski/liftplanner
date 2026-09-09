# Consistency review of `src`

Read directory by directory, file by file, after the layer split and the read
port work. Nothing was changed while reading.

Each finding carries a class:

- `[A]` a real divergence — two things with the same role have a different shape
- `[B]` repetition with no common home — a candidate for one helper
- `[C]` a detail, or a difference that is deliberate — noted, not necessarily changed
- `[!]` a probable defect or dead code

---

## `src/domain/workout`

**A1 `[B]` Four copies of the ordered-children logic.** `Workout::addExercise /
removeExercise / moveExercise / renumberExercises`, `Exercise::addSet /
removeSet / moveSet / renumberSets`, `TemplateExercise::addSet / … / renumberSets`
and `WorkoutTemplate::addExercise / … / renumberExercises` are the same six lines
four times, differing only in the element type and the member name. Insert
clamping, the out-of-range guard on remove, the `from == to` guard on move and the
renumbering loop are all repeated verbatim.

**A2 `[!]` Three empty `validate()` methods.** `Workout::validate`,
`Exercise::validate` and `Set::validate` have empty bodies and are called from the
constructors and the two `Set` factories. They are dead: validation actually
happens in `validationErrors()`, which nobody calls from a constructor. Either the
constructors should reject invalid values or the hook should go.

**A3 `[A]` Two of four entities have the `validate()` hook.** `WorkoutTemplate` and
`TemplateExercise` have `validationErrors()` and no `validate()`; `Workout` and
`Exercise` have both. Same role, different shape.

**A4 `[A]` Three ways to resolve a rest override.**
`Set::effectiveRestSeconds(exerciseDefault)`,
`TemplateExercise::effectiveRestSeconds(definitionDefault)` and
`Exercise::restSecondsForSet(index)` all answer "which rest applies here", with
three names and three shapes. The `-1 means inherit` convention is spelled out in
each of them.

**A5 `[C]` `Exercise::duplicateSet(index)` has no counterpart on
`TemplateExercise`.** The active workout can duplicate a set, a template cannot.
Probably deliberate, worth stating.

**A6 `[C]` `Exercise::setDefinitionId` / `clearDefinitionId` against
`TemplateExercise::setDefinitionId`.** A workout exercise may be unlinked from the
catalog, a template exercise may not. That is a real domain rule and the
asymmetry is right; it is nowhere written down.

## `src/domain/exercisecatalog`

**A7 `[A]` Child positions have two owners.** In the workout tree the entity owns
`position` and renumbers on every mutation. `ExerciseDefinition::addMuscle` does
not, and `MuscleInvolvementSerializer::toVariant(involvement, definitionId, i)`
takes the index from the save loop. The same concept is owned by the entity in
one topic and by the serializer in the other.

**A8 `[C]` `QStringList m_aliases` among `std::vector` children.**
`ExerciseDefinition` stores aliases as `QStringList` while every other collection
in the domain is a `std::vector`. Harmless, but it is the only one.

**A9 `[C]` Enum converters are all used.** Every `…ToString` / `…FromString` pair
in the domain has callers in `src` and in `tests`; there is nothing dead to
remove here.

## `src/infrastructure`

**A10 `[!]` Two migration mechanisms in the same class, one of them raw SQL.**
`SetRepositoryDb::createTable` and `WorkoutRepositoryDb::createTables` each run a
hand-written `PRAGMA table_info(...)`, scan the result for one column and, if it
is missing, execute a raw `ALTER TABLE` (and, for workouts, two raw `UPDATE`s).
Immediately below, `registerMigrations` does exactly that job through
`MigrationRunner` and the `AlterTable` builder. CLAUDE.md says all DDL and DML
goes through the query builders and forbids raw SQL, so this is the rule being
broken in the two places that also demonstrate the right way.

**A11 `[B]` `findByX` and `findByXs` are the same function twice.**
`SetRepositoryDb::findByExerciseId` / `findByExerciseIds`,
`ExerciseRepositoryDb::findByWorkoutId` / `findByWorkoutIds` and
`ExerciseDefinitionRepositoryDb::loadMuscles(one)` / `loadMuscles(many)` differ
only in `equals(id)` versus `in(ids)`. Six functions, three bodies.

**A12 `[B]` The row-to-entity loop is copied per repository.** `reserve`, iterate,
`push_back(Serializer::fromVariant(row))` appears in six places.

**A13 `[A]` Only the workout repository resolves a `save` by "does the row
exist".** `WorkoutRepositoryDb::save`, `ExerciseRepositoryDb::save` and
`SetRepositoryDb::save` each do `if (id != -1) { if (exists) update else insert }`,
which is what the toolkit's `upsert` is for. `ExerciseDefinitionRepositoryDb::save`
does something different again — it falls back to `findIdBySlug`.

**A14 `[C]` `whereclause.h` sits at the infrastructure root.** `addClause` and
`isNullClause` are toolkit-shaped helpers living in the app; they would be at home
in `dbtoolkit`.

## `src/application`

**A15 `[A]` Two services carry a read port, two do not.** `WorkoutService` and
`WorkoutTemplateService` take an aggregate port plus a row port;
`UserProfileService` and `ExerciseCatalogService` take one port. That is right —
only the first two have lists — but the constructors no longer look alike and
nothing says why.

**A16 `[C]` Every service splits `x()` returning `Task<T>` from `xCore()`
returning `Result<T>`, with `LIBS_TEST_FRIEND` to reach the core.** This is
consistent across all four services and is the shape to keep.

**A17 `[!]` `ExerciseCatalogService::archive` returns a `Task<bool>` that the
integration test drops**, producing a `-Wunused-result` warning on every build of
`tests/integration/exercise_catalog_test.cpp`.

## `src/ui/models`

**A18 `[A]` Sibling models expose different child operations.** `WorkoutModel`
has `addExercise` and `moveExercise` but no remove; `ExerciseModel` has `addSet`
and `removeSet` but no move. The two lists are edited by the same screens.

**A19 `[!]` `WorkoutModel::id` is declared `CONSTANT` but changes.** A workout
created in the editor has id `-1` until the save comes back and assigns one, so
any QML binding on `workout.id` keeps the stale value.

**A20 `[A]` The entity inside a model is only half authoritative.**
`WorkoutModel` holds a `Workout` *and* a `QList<ExerciseModel*>`; the entity's own
`exercises()` is stale and `toEntity()` rebuilds it. The same is true of
`ExerciseModel`. Nothing in the type says which half to trust.

**A21 `[A]` `WorkoutTemplateModel` notifies a `templateId` that never changes**
while `WorkoutModel::id` is `CONSTANT`. Two models, two answers for the same
question.

## `src/ui/viewmodels`

**A22 `[A]` Only two of six view models report loading.**
`WorkoutTemplateViewModel` and `ExerciseCatalogViewModel` have a `loading`
property and a private `setLoading`; `PlannedWorkoutViewModel`,
`WorkoutHistoryViewModel` and the others load asynchronously with no way for the
screen to know.

**A23 `[A]` `UserProfileViewModel` has no `errorOccurred`.** Every other view
model emits it. A failed profile save now reaches only the log — the screen still
emits `saved()`.

**A24 `[B]` Two shapes for "set a filter and reload".**
`WorkoutTemplateViewModel::setSearchText` trims, compares, emits and calls
`load()`; `ExerciseCatalogViewModel` has a private `applyFilterChange(QString&,
const QString&)` doing the same for five fields.

**A25 `[C]` `WorkoutTemplateViewModel` exposes `empty`, `ExerciseCatalogViewModel`
does not**, though both expose `count`.

**A26 `[A]` Two shapes for the tail of a task chain.** After the nodiscard work
every chain ends in an error handler, but some use
`.onError(this, …)` to reach a signal and some `.warnOnError(…)` to reach the log.
The rule — a chain that can tell the user uses `onError`, one that cannot uses
`warnOnError` — is followed but unwritten.

## `src/liftplannerapplication.cpp`, `src/main.cpp`

**A27 `[B]` `initialize()` is one method for two groups of objects.** It opens the
storage, builds four services on the worker thread and seven view models plus two
helpers on the main thread, in one body. A `createServices()` and a
`createViewModels()` would say what the two halves are.

**A28 `[B]` `drainWorker()` is duplicated verbatim in
`TestApplication::drain()`**, magic `64` included.

**A29 `[C]` The destructor resets `m_clipboardHelper` explicitly but not
`m_appInfo`.** Both are plain `unique_ptr` members with no cross-thread concern;
one is named, one is left to the implicit order.

**A30 `[C]` `main.cpp` carries two explanatory comments** in a codebase whose
CLAUDE.md says the code should explain itself. Also `"liftplanner.db"` and the
automation port `49200` are unnamed literals.

---

## Order of repair

`[A]`, `[B]` and `[!]` get fixed; `[C]` is recorded and left. In batches from the
domain upwards, tests and a commit after each:

| # | Batch | Findings | Done |
|---|---|---|---|
| 1 | dead validation hooks in the domain | A2, A3 | yes |
| 2 | one named policy for ordered children | A1 | yes |
| 3 | one rule for an inherited rest | A4 | yes |
| 4 | migrations through the runner, no raw SQL | A10 | yes |
| 5 | one shape for repository reads and saves | A11, A12, A13 | yes |
| 6 | one child API across the models | A18, A19, A21 | A19, A21; A18 left |
| 7 | one shape for the list view models | A22, A23, A24 | yes |
| 8 | composition root split by group | A27, A28 | yes, plus A29 |
| 9 | remaining small ones | A17, A7 | A17; A7 left |
| 10 | one authoritative half in the models | A20 | yes |

## Rules adopted while repairing

Filled in as each batch lands, so that a month from now the reason for the chosen
variant is written down rather than guessed.

**Validation is a query, not a hook (batch 1, A2/A3).** An entity answers
`validationErrors()` and `isValid()`; it never rejects a value in its
constructor. The caller decides what an invalid entity means — the editor shows
the errors, the importer collects them. All four entities now have that one
shape.

**Ordered children go through `Ordered` (batch 2, A1).** `domain/ordered.h`
owns insert clamping, the out-of-range guard, the `from == to` guard and the
renumbering loop for every `std::vector` of positioned children. An entity's
`addX / removeX / moveX` is a one-line call; anything the entity does *on top*
of the move — resetting the id of a duplicated set, renumbering grandchildren in
`normalizePositions` — stays in the entity.

**One sentinel for an inherited rest (batch 3, A4).**
`RestSeconds::inherited` is the `-1`, `RestSeconds::effective(override,
fallback)` is the rule. `Set` and `TemplateExercise` both spell their question
`effectiveRestSeconds(default)`. `Exercise::restSecondsForSet(index)` keeps its
own name because it answers a different question — it owns the default and takes
an index rather than receiving a fallback.

**Schema changes are migrations (batch 4, A10).** Nothing outside
`registerMigrations` may alter a table. A migration is idempotent because
`AlterTable` skips columns that already exist; a backfill guards on the value the
`ADD COLUMN` default just wrote, so it is safe to re-run. `AlterTable::execute`
is checked with `!= 0`, `Update::execute` with `>= 0` — it returns the row count,
and matching nothing is a success.

**Repositories read through `findBy(Where)` and write through `upsert` (batch 5,
A11/A12/A13).** The one-id and many-ids finders differ only in their `Where`, so
they share a private `findBy`; rows become entities through
`DbRows::toEntities`. `save` is one `upsert` call — the toolkit already knows how
to insert or update on the primary key. The catalog keeps its `findIdBySlug`
step: the slug is a natural key, so resolving it is a different question from
choosing between insert and update.

**Model ids notify (batch 6, A19/A21).** `WorkoutModel::id` and
`WorkoutTemplateModel::templateId` both notify; no model id is `CONSTANT`. A save
writes the assigned id back into the model, so a binding on `workout.id` cannot
hold a stale `-1`. The write-back is skipped while the view model is being
destroyed, because a `then(this, …)` continuation asserts that its context object
is still alive when it runs.

**Every list view model reports `loading` (batch 7, A22).** A view model that
fills a list from an asynchronous service has `loading` with a private
`setLoading`, set before the call and cleared in both the continuation and the
error handler. That is all four list view models — planned, history, templates,
catalog. The editor, the active workout and the profile are not lists and do not
have it. `LoadingOverlay` from `libs/qml/app` is the component a screen binds it
to.

**A chain that can tell the user uses `onError` (batch 7, A23/A26).** A view
model with an `errorOccurred` signal ends every chain in `.onError(this, …)`;
`warnOnError` is for chains with nobody to tell. `UserProfileViewModel` now has
the signal, and its `saved()` is emitted from the continuation rather than
before the save has happened.

**Trimming a filter is one rule (batch 7, A24).** `FilterField::apply(target,
value)` trims, compares and assigns, and answers whether anything changed. Each
view model keeps its own signal — the catalog has one `filtersChanged` for five
fields, the template list has `searchTextChanged` for one.

**The composition root names its two halves (batch 8, A27/A28/A29).**
`initialize()` calls `createServices()` — everything built on the worker thread,
returning whether the storage opened — and `createViewModels()` — everything
built on the main thread. Draining the worker queue is `BackendWorker::drain()`
in libs, so the app and `TestApplication` share it instead of each keeping the
magic 64. The destructor names every member it releases, `m_appInfo` included.

**A task's result is consumed, never dropped (batch 9, A17).** The integration
test now reads what `archive` returned instead of discarding the `Task<bool>`
and warning on every build.

**A7 left as it is.** A workout's exercises and sets each own their `position`
because each is a row with an identity that survives a save — they are updated in
place, moved and renumbered. A definition's muscles are a value list that
`saveMuscles` deletes and reinserts whole, so the index in the vector *is* the
position; giving `MuscleInvolvement` a `position` field would add a second source
of truth for the same fact and break equality, which compares what a muscle
involvement is, not where it sits.

**A model keeps a record, never a second copy of its children (batch 10, A20).**
`WorkoutModel` and `ExerciseModel` still hold an entity, but only for its
scalars: `recordOf()` strips the collection as the entity enters the
constructor, so `m_record` carries no children and the child models are the only
answer. `toEntity()` is then the same two lines in both — copy the record, add
what the child models say — instead of `WorkoutModel` listing seven setters by
hand, which silently loses any field added to `Workout` later. `previousSummary`
and `previousDate` stay outside the record, because they are what the screen
knows about an exercise and not what the database stores.

The mutable `Workout::exercises()` and `Exercise::sets()` remain, because
`WorkoutEditorViewModel` and `ActiveWorkoutViewModel` edit an entity in place
outside the models. Closing that needs the bigger split — a read-only projection
for the lists and the editor, a mutable model for the active workout — which
would also remove the editor's double state (`Workout` plus a `WorkoutModel*`
rebuilt on every change).

**A18 left as it is.** `WorkoutModel` has `addExercise` and `moveExercise`,
`ExerciseModel` has `addSet` and `removeSet`. Those are exactly the operations
the active workout screen performs — the editor works on the entity, not on the
models. Adding the missing two would be API nothing calls, which is a worse
divergence than the one it removes.
