# Plan refaktoringu LiftPlanner → Clean Architecture (DDD)

## Zasada przewodnia
> Każdy krok kończy się **kompilującą i działającą aplikacją**. Żadna funkcjonalność nie zostaje usunięta ani zepsuta.

---

## Faza 0 — Przygotowanie infrastruktury

| # | Zadanie | Opis |
|---|---------|------|
| 0.1 | **Podłączyć submodule `libs`** | Dodać `add_subdirectory(libs)` w root `CMakeLists.txt`. Aktualnie `libs/` istnieje jako submodule, ale nie jest włączony do buildu. |
| 0.2 | **Zlinkować `dbtoolkit` i `app_utils`** | W `src/CMakeLists.txt` dodać `target_link_libraries(... dbtoolkit app_utils)`. Na razie nic z nich nie używamy — upewnić się, że build przechodzi. |
| 0.3 | **Usunąć duplikaty z `src/lib/`** | `src/lib/` zawiera `filewatcher.h/.cpp` — ten sam kod już istnieje w `libs/qmllive/`. Zamienić include'y na wersję z libs, usunąć `src/lib/`. |
| 0.4 | **Usunąć duplikat `SvgColorizer`** | `SvgColorizer` istnieje zarówno w `src/utils/` jak i w `libs/utils/`. Przejść na wersję z `libs/utils`, usunąć lokalną kopię. |
| 0.5 | **Podnieść standard C++ do C++20** | W root `CMakeLists.txt` zmienić `CMAKE_CXX_STANDARD` z 17 na 20. |
| 0.6 | **Zweryfikować build** | Cały projekt musi się kompilować i działać tak samo jak przed zmianami. |

---

## Faza 1 — Wprowadzenie warstwy `core/`

| # | Zadanie | Opis |
|---|---------|------|
| 1.1 | **Stworzyć `src/core/`** z `CMakeLists.txt` | Nowa statyczna biblioteka `core` — na razie pusta. Dodać `add_subdirectory(core)` w `src/CMakeLists.txt`. |
| 1.2 | **Dodać `core/types.h`** | Przenieść/stworzyć wspólne typy: `SortDirection`, ewentualnie inne enumy. |
| 1.3 | **Dodać `core/result.h`** | Zaimplementować klasę `Result<T>` i `Result<void>` — na razie nieużywane, ale gotowe. |
| 1.4 | **Przenieść `AppStorage` do `core/storage/`** | Przenieść obecny `src/storage/` do `src/core/storage/`. Zaktualizować include'y. Na razie bez zmian w logice. |
| 1.5 | **Dodać `core/events/`** | Zaimplementować `EventBase` + `EventBus` — na razie nieużywany, ale gotowy na późniejszą integrację. |
| 1.6 | **Zweryfikować build** | Aplikacja musi działać identycznie. |

---

## Faza 2 — Ekstrakcja modułu `workout` (domain layer)

| # | Zadanie | Opis |
|---|---------|------|
| 2.1 | **Stworzyć `src/modules/workout/`** | Struktura: `domain/entities/`, `domain/repositories/`, `application/`, `infrastructure/database/`, `infrastructure/serializers/`. |
| 2.2 | **Wyekstrahować encje domenowe** | Z obecnych QObject-owych modeli (`Workout`, `Exercise`, `Set` w `src/models/`) wyciągnąć **czyste klasy `final`** bez QObject, bez serializacji. Umieścić w `modules/workout/domain/entities/`. Obecne modele QObject zostawiamy tymczasowo — encje domenowe to nowe, niezależne klasy. |
| 2.3 | **Stworzyć interfejsy repozytoriów** | Czyste wirtualne interfejsy: `WorkoutRepository`, `ExerciseRepository`, `SetRepository` w `modules/workout/domain/repositories/`. |
| 2.4 | **Stworzyć Query objects** | `WorkoutQuery`, `ExerciseQuery`, `SetQuery` z fluent API w `domain/repositories/`. |
| 2.5 | **Stworzyć serializery** | `WorkoutSerializer`, `ExerciseSerializer`, `SetSerializer` w `infrastructure/serializers/` — logika `toVariantMap`/`fromVariantMap` wyciągnięta z obecnych modeli. |
| 2.6 | **Stworzyć implementacje repozytoriów na `dbtoolkit`** | `WorkoutRepositoryDb`, `ExerciseRepositoryDb`, `SetRepositoryDb` w `infrastructure/database/` — korzystające z nowego `DbRepository`/`DbStorage` z libs. Zastępują stary `GenericRepository` + `ModelRepository`. |
| 2.7 | **Zweryfikować build** | Nowe klasy kompilują się obok starych. Nic jeszcze ich nie wywołuje. |

---

## Faza 3 — Migracja serwisów na czystą architekturę

| # | Zadanie | Opis |
|---|---------|------|
| 3.1 | **Rozdzielić Service od ViewModel** | Każdy z obecnych serwisów (`ActiveWorkoutService`, `WorkoutService`, `WorkoutHistoryService`) jest QObject-em wystawionym do QML. Rozdzielić na: (a) **czysty Service** (plain C++, zwraca `Result<T>`) w `modules/workout/application/`, (b) **ViewModel** (QObject z Q_PROPERTY/Q_INVOKABLE) w `src/ui/viewmodels/`. |
| 3.2 | **Service korzysta z nowych repozytoriów** | Serwisy dostają referencje do interfejsów `*Repository`, nie do konkretnych klas DB. |
| 3.3 | **ViewModel deleguje do Service** | ViewModel trzyma wskaźnik do Service, wywołuje metody i mapuje `Result<T>` na Q_PROPERTY/sygnały. |
| 3.4 | **Podpiąć ViewModele w `main.cpp`** | Zastąpić rejestrację serwisów jako QML singletonów rejestracją ViewModeli. QML nie powinien odczuć zmiany (te same property/invokable). |
| 3.5 | **Zweryfikować build i działanie** | Aplikacja działa identycznie, ale wewnętrznie jest rozdzielona. |

---

## Faza 4 — Klasa bootstrap `LiftPlannerApplication`

| # | Zadanie | Opis |
|---|---------|------|
| 4.1 | **Stworzyć `core/app/liftplannerapplication.h/.cpp`** | Klasa posiadająca cały graf obiektów: `AppStorage`, `EventBus`, serwisy, ViewModele. |
| 4.2 | **Przenieść logikę z `main.cpp`** | Obecny `main.cpp` tworzy serwisy, storage, rejestruje QML ręcznie. Przenieść tę logikę do `LiftPlannerApplication::initialize()` i `registerQmlTypes()`. |
| 4.3 | **Zaktualizować `AppStorage`** | `AppStorage` powinien korzystać z `DbStorage` z libs/dbtoolkit zamiast bezpośrednio z `QSqlDatabase`. Powinien tworzyć nowe `*RepositoryDb`. |
| 4.4 | **Użyć `QmlRegistrator` z libs/utils** | Zastąpić ręczną rejestrację QML użyciem `QmlRegistrator`. |
| 4.5 | **Uprościć `main.cpp`** | `main.cpp` powinien wyglądać jak w sekcji 10.3 dokumentu architektonicznego. |
| 4.6 | **Zweryfikować build i działanie** | |

---

## Faza 5 — Sprzątanie i usunięcie starego kodu

| # | Zadanie | Opis |
|---|---------|------|
| 5.1 | **Usunąć stare modele QObject** | Z `src/models/` — zastąpione encjami domenowymi + ViewModelami. |
| 5.2 | **Usunąć stare repozytoria** | Z `src/repositories/` — zastąpione nowymi implementacjami w `modules/*/infrastructure/database/`. |
| 5.3 | **Usunąć stare serwisy** | Z `src/services/` — zastąpione serwisami w `modules/*/application/` + ViewModelami w `ui/viewmodels/`. |
| 5.4 | **Usunąć `src/utils/` duplikaty** | Wszystko co jest w libs — usunąć lokalne kopie. Pozostawić tylko to, co jest specyficzne dla LiftPlannera. |
| 5.5 | **Usunąć stary `GenericRepository`/`ModelRepository`** | Z `src/repositories/` — zastąpione przez `DbRepository` z dbtoolkit. |
| 5.6 | **Wyczyścić `src/CMakeLists.txt`** | Zmienić z monolitycznego GLOBa na linkowanie modułów: `core`, `workout_module`, itp. |
| 5.7 | **Zweryfikować build i działanie** | |

---

## Faza 6 — Modularyzacja UI (QML)

| # | Zadanie | Opis |
|---|---------|------|
| 6.1 | **Użyć `Themed.Components` z libs** | Zastąpić niestandardowe komponenty QML (np. `PrimaryButton`) komponentami z `libs/qmlcomponents/` (`ThemedButton`, `ThemedCard`, itd.). |
| 6.2 | **Użyć `Themed.Icons` z libs** | Zastąpić lokalne SVG ikony z `src/ui/icons/` ikonami z `libs/icons/` tam gdzie to możliwe. Ikony specyficzne dla LiftPlannera zostawić lokalnie. |
| 6.3 | **Zweryfikować build i wygląd UI** | |

---

## Faza 7 — Testy

| # | Zadanie | Opis |
|---|---------|------|
| 7.1 | **Dodać submodule Google Test** | `git submodule add https://github.com/google/googletest.git tests/third_party/googletest` |
| 7.2 | **Stworzyć `tests/CMakeLists.txt`** | Z `enable_testing()` i `gtest_discover_tests()`. |
| 7.3 | **Dodać testy encji** | Testy jednostkowe dla `Workout`, `Exercise`, `Set` w `tests/workout/domain/entities/`. |
| 7.4 | **Dodać testy serwisów z mockami** | Mockowane repozytoria → testy logiki serwisów w `tests/workout/application/`. |
| 7.5 | **Dodać testy integracyjne repozytoriów** | Testy z prawdziwą bazą SQLite in-memory w `tests/workout/infrastructure/database/`. |

---

## Docelowa struktura po refaktoringu

```
src/
├── CMakeLists.txt
├── main.cpp
├── core/
│   ├── CMakeLists.txt
│   ├── types.h
│   ├── result.h
│   ├── app/
│   │   └── liftplannerapplication.h/.cpp
│   ├── storage/
│   │   └── appstorage.h/.cpp
│   └── events/
│       ├── eventbase.h/.cpp
│       └── eventbus.h/.cpp
├── modules/
│   └── workout/
│       ├── CMakeLists.txt
│       ├── domain/
│       │   ├── entities/
│       │   │   ├── workout.h/.cpp
│       │   │   ├── exercise.h/.cpp
│       │   │   └── set.h/.cpp
│       │   └── repositories/
│       │       ├── workoutrepository.h
│       │       ├── exerciserepository.h
│       │       ├── setrepository.h
│       │       ├── workoutquery.h/.cpp
│       │       ├── exercisequery.h/.cpp
│       │       └── setquery.h/.cpp
│       ├── application/
│       │   ├── activeworkoutservice.h/.cpp
│       │   ├── workoutservice.h/.cpp
│       │   └── workouthistoryservice.h/.cpp
│       └── infrastructure/
│           ├── database/
│           │   ├── workoutrepositorydb.h/.cpp
│           │   ├── exerciserepositorydb.h/.cpp
│           │   └── setrepositorydb.h/.cpp
│           └── serializers/
│               ├── workoutserializer.h/.cpp
│               ├── exerciseserializer.h/.cpp
│               └── setserializer.h/.cpp
└── ui/
    ├── viewmodels/
    │   ├── activeworkoutviewmodel.h/.cpp
    │   ├── workoutviewmodel.h/.cpp
    │   └── workouthistoryviewmodel.h/.cpp
    ├── models/          (QAbstractListModel-y jeśli potrzebne)
    └── qml/
        └── (obecne pliki QML)
```

---

## Priorytety i kolejność

Fazy **0 → 1 → 2 → 3** są najważniejsze i powinny być realizowane sekwencyjnie. Faza 0 (podłączenie libs) jest **bezpieczna i szybka** — można ją zrobić od razu. Fazy 4-7 można robić w dowolnej kolejności po zakończeniu fazy 3.
