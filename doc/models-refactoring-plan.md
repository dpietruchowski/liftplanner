# Plan: Usunięcie QObject modeli — przejście na wzorzec Entity + ListModel

## Cel

Wyeliminować warstwę pośrednią `WorkoutModel`, `ExerciseModel`, `SetModel` (QObject),
która duplikuje encje domenowe (`Workout`, `Exercise`, `Set`) i wymusza boilerplate
konwersji w `AppDbStorage`. Wzorzec docelowy: **fillin-app** — encje domenowe +
`QAbstractListModel` do wyświetlania list + płaskie `Q_PROPERTY` na ViewModelach.

## Obecny przepływ danych

```
QML  →  ViewModel (QObject)
           ↕ WorkoutModel* / ExerciseModel* / SetModel*
         AppDbStorage
           ↕ populateModel / fromModel (6 metod konwersji)
         Repository  →  Entity
```

**Problem:** Każde nowe pole = zmiana w 3 miejscach (Entity, Model, mapper).

## Docelowy przepływ danych

```
QML  →  ViewModel (flat Q_PROPERTY + ListModel*)
           ↕ Workout / Exercise / Set (encje domenowe z dziećmi)
         AppDbStorage
           ↕ bezpośrednio
         Repository  →  Entity
```

---

## Faza 1: Wzbogacenie encji + nowe modele listowe

### 1.0 Wzbogacenie encji domenowych (aggregate roots)
- `Set` — bez zmian (liść)
- `Exercise` — dodać `std::vector<Set> m_sets` + metody:
  `addSet(Set)`, `removeSet(int index)`, `sets()`, `isCompleted()`,
  `setsToString()` — logika domenowa na encji zamiast na modelu QObject
- `Workout` — dodać `std::vector<Exercise> m_exercises` + metody:
  `addExercise(Exercise)`, `removeExercise(int index)`, `exercises()`,
  `isCompleted()`

### 1.1 `SetListModel` (QAbstractListModel)
- Opakowuje `std::vector<Set>` (referencja do vectora z Exercise)
- Role: `RepetitionsRole`, `WeightRole`, `CompletedRole`, `IndexRole`
- Metody mutujące: `updateSet(int index, Set)`, `addSet(Set)`, `removeSet(int index)`
- Sygnały `dataChanged` przy edycji pola

### 1.2 `ExerciseListModel` (QAbstractListModel)
- Opakowuje `std::vector<Exercise>` (referencja do vectora z Workout)
- Role: `NameRole`, `DescriptionRole`, `RestSecondsRole`, `YoutubeLinkRole`,
  `CompletedRole`, `SetsStringRole`
- `Q_INVOKABLE SetListModel* setsModel(int index)` — zwraca model setów dla danego ćwiczenia
  (leniwie tworzony, cache w QHash)
- Posiada na własność child `SetListModel`-e

### 1.3 `WorkoutListModel` (QAbstractListModel)
- Opakowuje `std::vector<Workout>`
- Role: `NameRole`, `StartedTimeRole`, `EndedTimeRole`, `CreatedTimeRole`,
  `CompletedRole`, `IdRole`
- `Q_INVOKABLE ExerciseListModel* exercisesModel(int index)` — model ćwiczeń dla danego
  treningu (leniwie tworzony, cache w QHash)

### 1.4 `src/ui/models/CMakeLists.txt`
- Zbiera pliki, dodaje do `PROJECT_SOURCES`

---

## Faza 2: Refaktor `AppDbStorage`

### Usunąć:
- `workoutFromModel()`, `exerciseFromModel()`, `setFromModel()`
- `populateWorkoutModel()`, `populateExerciseModel()`, `populateSetModel()`
- Inkludy `models/workoutmodel.h`, `models/exercisemodel.h`, `models/setmodel.h`

### Zmienić:
- `loadAllWorkouts(condition)` → zwraca `std::vector<WorkoutData>`
  (ładuje workout + exercises + sets za jednym razem)
- `loadWorkout(int workoutId)` → zwraca `WorkoutData`
- `saveWorkout(const WorkoutData&)` → zapisuje workout + exercises + sets, zwraca id
- `removeWorkouts(condition)` — bez zmian (operuje na repo)

---

## Faza 3: Serializacja JSON dla encji

### 3.1 `src/ui/models/workoutjson.h/.cpp`
Funkcje pomocnicze do serializacji/deserializacji `WorkoutData` ↔ JSON:

- `QJsonObject workoutDataToJson(const WorkoutData&, SerializationMode)`
- `WorkoutData workoutDataFromJson(const QJsonObject&)`
- `QJsonArray workoutListToJson(const std::vector<WorkoutData>&, SerializationMode)`

Zastępują `WorkoutModel::toJson/fromJson`. `SerializationMode` przeniesiony z `models/types.h`.

### 3.2 Przenieść `SerializationMode` do `src/ui/models/workoutdata.h`

---

## Faza 4: Refaktor `RoutineViewModel`

### Było:
```cpp
QList<WorkoutModel*> m_workouts;
Q_PROPERTY(QList<WorkoutModel*> workouts ...)
Q_PROPERTY(WorkoutModel* nextWorkout ...)
```

### Będzie:
```cpp
std::vector<WorkoutData> m_workouts;
WorkoutListModel* m_workoutListModel;

Q_PROPERTY(WorkoutListModel* workouts READ workoutsModel NOTIFY workoutsChanged)
Q_PROPERTY(QString nextWorkoutName READ nextWorkoutName NOTIFY nextWorkoutChanged)
```

### Zmiany w metodach:
- `loadAllWorkouts()` — pobiera `std::vector<WorkoutData>` z AppDbStorage, ustawia na modelu
- `importWorkoutsFromJson(jsonData)` — parsuje JSON do `std::vector<WorkoutData>` via workoutjson.h
- `generateGptPrompt()` — serializuje via workoutjson.h
- `nextWorkout` — zamiast zwracać pointer, eksponuje `nextWorkoutName` (QString) i
  `Q_INVOKABLE WorkoutData` albo indeks potrzebny do `startWorkout(int index)`
- Nowe: `Q_INVOKABLE void startWorkoutByIndex(int index)` — emituje sygnał z WorkoutData

---

## Faza 5: Refaktor `WorkoutHistoryViewModel`

### Było:
```cpp
QList<WorkoutModel*> m_workouts;
DECLARE_PROPERTY(QList<WorkoutModel*>, workouts, setWorkouts)
Q_PROPERTY(WorkoutModel* lastWorkout ...)
```

### Będzie:
```cpp
std::vector<WorkoutData> m_workouts;
WorkoutListModel* m_workoutListModel;

Q_PROPERTY(WorkoutListModel* workouts READ workoutsModel NOTIFY workoutsChanged)
Q_PROPERTY(QString lastWorkoutName READ lastWorkoutName NOTIFY lastWorkoutChanged)
```

### Zmiany:
- `loadAllWorkouts()` — pobiera `vector<WorkoutData>`, ustawia na modelu
- `lastWorkout` → `lastWorkoutName` (QString), `lastWorkoutDate` (QDateTime)
- `deleteWorkout(int workoutId)` zamiast `deleteWorkout(WorkoutModel*)`
- `saveWorkout(const WorkoutData&)` zamiast `saveWorkout(WorkoutModel*)`
- `recentWorkoutsToJson()` — serializuje przez workoutjson.h

---

## Faza 6: Refaktor `ActiveWorkoutViewModel`

Najbardziej złożony — obsługuje mutację danych i nawigację.

### Było:
```cpp
DECLARE_PROPERTY(WorkoutModel*, currentWorkout, setCurrentWorkout)
DECLARE_PROPERTY(ExerciseModel*, currentExercise, setCurrentExercise)
DECLARE_PROPERTY(SetModel*, currentSet, setCurrentSet)
```

### Będzie:
```cpp
WorkoutData m_workoutData;                    // bieżący trening (sklonowany)
ExerciseListModel* m_exerciseListModel;
int m_currentExerciseIndex{-1};
int m_currentSetIndex{-1};

// Flat properties dla bieżącego stanu
Q_PROPERTY(bool isActive ...)
Q_PROPERTY(QString workoutName ...)
Q_PROPERTY(bool workoutCompleted ...)
Q_PROPERTY(ExerciseListModel* exercises ...)
Q_PROPERTY(QString currentExerciseName ...)
Q_PROPERTY(int currentExerciseRestSeconds ...)
Q_PROPERTY(int currentExerciseIndex ...)
Q_PROPERTY(int currentSetIndex ...)
Q_PROPERTY(int currentSetRepetitions ...)
Q_PROPERTY(int currentSetWeight ...)
Q_PROPERTY(bool currentSetCompleted ...)
```

### Zmiany w metodach:
- `startWorkout(const WorkoutData&)` — klonuje WorkoutData, buduje ExerciseListModel
- `completeCurrentSet()` — ustawia `sets[idx].setCompleted(true)`, emituje dataChanged
- `navigateToNext/Previous()` — zmienia indeksy
- `duplicateSet(int exerciseIdx, int setIdx)` — kopiuje Set, dodaje do vectora, odświeża model
- `removeSet(int exerciseIdx, int setIdx)` — analogicznie
- `updateSetRepetitions(int exIdx, int setIdx, int value)` — mutuje Set, odświeża
- `updateSetWeight(int exIdx, int setIdx, int value)` — mutuje Set, odświeża
- `saveCurrentWorkout()` — serializuje WorkoutData do JSON (plik)
- `loadCurrentWorkout()` — deserializuje z JSON
- `endWorkout()` — emituje `workoutCompleted(WorkoutData)` do zapisu w historii

---

## Faza 7: Aktualizacja QML

### 7.1 `ScreenHome.qml`
- `RoutineService.nextWorkout.name` → `RoutineService.nextWorkoutName`
- `WorkoutHistoryService.lastWorkout.name` → `WorkoutHistoryService.lastWorkoutName`
- `ActiveWorkoutService.startWorkout(RoutineService.nextWorkout)` →
  `RoutineService.startNextWorkout()` (ViewModel emituje sygnał → ActiveWorkoutService)

### 7.2 `ScreenRoutines.qml`
- `RoutineService.workouts` → `RoutineService.workouts` (ale teraz to WorkoutListModel*)
- Delegate RoutineItem: `workout: modelData` → dostęp przez role

### 7.3 `RoutineItem.qml`
- `workout.name` → `model.name`
- `workout.exercises` → `RoutineService.workouts.exercisesModel(index)`
- `startWorkoutRequest(workout)` → `startWorkoutRequest(index)`

### 7.4 `WorkoutItem.qml`
- `workout.name` → `model.name`
- `workout.startedTime` → `model.startedTime`
- `workout.exercises` → `WorkoutHistoryService.workouts.exercisesModel(index)`
- `deleteWorkout(workout)` → `deleteWorkout(model.id)`

### 7.5 `ExerciseItem.qml`
- `exercise.name` → `model.name`
- `exercise.setsToString()` → `model.setsString`

### 7.6 `ScreenActiveWorkout.qml`
- `ActiveWorkoutService.currentWorkout.name` → `ActiveWorkoutService.workoutName`
- `ActiveWorkoutService.currentWorkout.completed` → `ActiveWorkoutService.workoutCompleted`
- `ActiveWorkoutService.currentWorkout.exercises` → `ActiveWorkoutService.exercises`
- `ActiveWorkoutService.currentExercise.restSeconds` → `ActiveWorkoutService.currentExerciseRestSeconds`
- Edycja setów: `setData.repetitions = newValue` →
  `ActiveWorkoutService.updateSetRepetitions(exerciseIndex, setIndex, newValue)`

### 7.7 `ActiveWorkoutExerciseItem.qml`
- `exercise.name` → `model.name`
- `exercise.completed` → `model.completed`
- `exercise.sets` → `ActiveWorkoutService.exercises.setsModel(index)`
- `ActiveWorkoutService.currentExercise === exercise` →
  `ActiveWorkoutService.currentExerciseIndex === index`

### 7.8 `ActiveWorkoutSetItem.qml`
- `setData.repetitions` → `model.repetitions`
- `setData.weight` → `model.weight`
- `setData.completed` → `model.completed`
- `ActiveWorkoutService.currentSet === setData` →
  `ActiveWorkoutService.currentSetIndex === model.index`
- `ActiveWorkoutService.duplicateSet(modelData)` →
  `ActiveWorkoutService.duplicateSet(exerciseIndex, index)`
- `ActiveWorkoutService.removeSet(modelData)` →
  `ActiveWorkoutService.removeSet(exerciseIndex, index)`

---

## Faza 8: Usunięcie starych modeli

- Usunąć `src/models/workoutmodel.h/.cpp`
- Usunąć `src/models/exercisemodel.h/.cpp`
- Usunąć `src/models/setmodel.h/.cpp`
- Usunąć `src/models/types.h`
- Usunąć `src/models/CMakeLists.txt`
- Z `src/CMakeLists.txt` usunąć `add_subdirectory(models)`
- Z `src/utils/serializationutils.h` usunąć makra `DECLARE_MODEL_PROPERTY` i
  `DECLARE_MODEL_LIST_PROPERTY`, zostawić `DECLARE_PROPERTY`

---

## Faza 9: CMake & Build

- Dodać `add_subdirectory(ui/models)` w `src/CMakeLists.txt`
- Zbudować, poprawić błędy kompilacji
- Uruchomić testy (63 testy encji/serializerów/repozytoriów — nie powinny być dotknięte)
- Przetestować manualne (QML)

---

## Podsumowanie zmian

| Komponent | Przed | Po |
|-----------|-------|----|
| Encje domenowe | czyste C++ (bez zmian) | bez zmian ✓ |
| QObject modele | WorkoutModel, ExerciseModel, SetModel | **USUNIĘTE** |
| Konwersja w AppDbStorage | 6 metod populate/from | **USUNIĘTE** |
| Listy w QML | QQmlListProperty + QObject* | QAbstractListModel + role |
| Bieżący element w QML | pointer WorkoutModel* | flat Q_PROPERTY (int, QString) |
| Identyfikacja elementu | porównanie wskaźników | porównanie indeksów |
| Serializacja JSON | toJson/fromJson na modelach | workoutjson.h (wolne funkcje) |
| Klonowanie | WorkoutModel::clone() | kopiowanie WorkoutData (value type) |

### Nowe pliki:
- `src/ui/models/workoutdata.h`
- `src/ui/models/setlistmodel.h/.cpp`
- `src/ui/models/exerciselistmodel.h/.cpp`
- `src/ui/models/workoutlistmodel.h/.cpp`
- `src/ui/models/workoutjson.h/.cpp`
- `src/ui/models/CMakeLists.txt`

### Usunięte pliki:
- `src/models/workoutmodel.h/.cpp`
- `src/models/exercisemodel.h/.cpp`
- `src/models/setmodel.h/.cpp`
- `src/models/types.h`
- `src/models/CMakeLists.txt`
