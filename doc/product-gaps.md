# Product gaps

Backlog produktowy LiftPlannera. Każda pozycja opisuje, czego brakuje
użytkownikowi na siłowni, i wskazuje miejsce w kodzie, z którego to wynika.
Kolejność sekcji „Open" to kolejność ważności.

## Open

### G2 — Wybór dnia zaplanowanego treningu
- **Dla kogo/po co:** aplikacja nazywa się planerem, a każdy nowo utworzony
  trening ląduje na „teraz". Nie da się rozłożyć tygodnia na poniedziałek,
  środę i piątek.
- **Dowód:** `MainView.qml:54` woła `WorkoutEditorViewModel.createNew("", new
  Date())`, a `MainView.qml:47` — `startFromTemplate(templateId, new Date())`.
  `ScreenWorkoutEditor.qml` (118 linii) nie ma żadnej kontrolki daty, choć
  `plannedTime` jest zapisywalną właściwością view modelu
  (`workouteditorviewmodel.h:19`). W efekcie `PlannedWorkoutItem.qml:13`
  wyświetla dla wszystkich treningów dzisiejszą datę.
- **Zakres:** M
- **Gotowe, gdy:** w edytorze treningu da się ustawić dzień; po zapisie kafel na
  liście zaplanowanych pokazuje wybrany dzień, a nie dzisiejszy, i ta data
  przeżywa restart aplikacji.

### G3 — Usunięcie zaplanowanego treningu
- **Dla kogo/po co:** import z AI dokłada treningi do listy; nietrafiony plan
  albo trening, na który się nie poszło, zostaje na ekranie na zawsze.
- **Dowód:** `PlannedWorkoutItem.qml` ma tylko przycisk startu, podczas gdy
  bliźniaczy `WorkoutItem.qml:29-38` (historia) ma już akcję usunięcia opartą
  o `WorkoutHistoryViewModel.deleteWorkout`, która działa na dowolnym treningu.
  Jedyne masowe wyjście to `WorkoutService::removeAllPlannedWorkouts`
  (`src/application/workout/workoutservice.h:46`) — bez wejścia z UI.
- **Zakres:** S
- **Gotowe, gdy:** kafel zaplanowanego treningu ma akcję usunięcia z
  potwierdzeniem; po potwierdzeniu trening znika z sekcji PLANNED i nie wraca po
  restarcie aplikacji.

### G4 — „Start workout" ignoruje wybór na bębnie
- **Dla kogo/po co:** użytkownik przewija bęben na ekranie głównym, wybiera
  konkretny trening i naciska duży przycisk startu — a rusza inny. To dokładnie
  ten rodzaj zachowania, po którym przestaje się ufać aplikacji.
- **Dowód:** `WorkoutDrum.qml:24-28` — kliknięcie pozycji zmienia tylko
  `currentIndex`. `ScreenHome.qml:145-153` — `startWorkout()` startuje zawsze
  `PlannedWorkoutViewModel.nextWorkout`, nigdzie nie czyta stanu bębna. Bęben
  potrafi pokazać też „next planned" (`ScreenHome.qml:24`) i „last workout"
  (`ScreenHome.qml:28`).
- **Zakres:** S
- **Gotowe, gdy:** przy dwóch zaplanowanych treningach wybranie na bębnie tego
  drugiego i naciśnięcie „Start workout" uruchamia ten drugi (ekran aktywnego
  treningu ma jego nazwę w tytule); dla pozycji, której nie da się wystartować
  (miniony trening), przycisk nie startuje cudzego treningu.

### G5 — Edytor serii w aktywnym treningu nie daje o sobie znać
- **Dla kogo/po co:** zmiana ciężaru albo powtórzeń w trakcie serii to
  najczęstsza czynność na siłowni, a jedyny sposób jej otwarcia to długie
  przytrzymanie wiersza, o którym nic nie informuje.
- **Dowód:** `SetRow.qml:63-67` — `HoldToRevealArea` z `onHeld: root.expanded =
  !root.expanded` i żadnej ikony ani podpowiedzi; w edytorze treningu ten sam
  komponent dostaje `expanded: true` na sztywno
  (`EditorExerciseItem.qml:80`), więc problem dotyczy tylko aktywnego treningu.
  Pozycja odnotowana też w `doc/todo.md:89` jako „do decyzji".
- **Zakres:** S
- **Gotowe, gdy:** w aktywnym treningu wiersz serii ma widoczny sygnał, że da
  się go rozwinąć, i da się go rozwinąć bez przytrzymywania; steppery REPS/KG
  oraz akcje duplikuj/usuń są po tym widoczne.

### G6 — Czas przerwy jest niewidoczny i nie do ustawienia
- **Dla kogo/po co:** przerwa decyduje o charakterze treningu (siła vs
  hipertrofia), a timer sam odlicza wartość, której użytkownik nigdzie nie widzi
  ani nie może zmienić inaczej niż klikając ±15 s w trakcie odliczania.
- **Dowód:** `ExerciseModel` wystawia `restSeconds`
  (`src/ui/models/exercisemodel.h:15`), ale ciąg „restSeconds" nie występuje w
  żadnym pliku pod `src/ui/qml/`. `WorkoutEditorViewModel::setExerciseRest`
  (`workouteditorviewmodel.h:51`) nie jest wywoływany z QML —
  `EditorExerciseItem.qml` ma tylko nazwę, strzałki kolejności, usunięcie i serie.
- **Zakres:** M
- **Gotowe, gdy:** w edytorze treningu przy ćwiczeniu widać czas przerwy i da
  się go zmienić; po zapisaniu i wystartowaniu treningu timer po ukończeniu
  serii tego ćwiczenia odlicza ustawioną wartość.

### G8 — Brak podsumowania po zakończonym treningu
- **Dla kogo/po co:** trening kończy się wyrzuceniem na ekran główny bez ani
  jednego zdania o tym, co się właśnie zrobiło. Zamknięcie sesji to najlepszy
  moment na nagrodę i jedyny, w którym liczby jeszcze kogoś obchodzą.
- **Dowód:** `ScreenActiveWorkout.qml:161-167` — `onWorkoutCompleted` robi
  wyłącznie `stackView.replace(homeScreen)`. Dane do podsumowania już są
  liczone: `WorkoutService::TrainingTotals` (objętość, czas, dystans) i
  `topExercises` (`src/application/workout/workoutservice.h:19-32,51-52`).
- **Zakres:** M
- **Gotowe, gdy:** po zakończeniu treningu pojawia się ekran lub panel z czasem
  trwania i liczbą ukończonych serii tej sesji, zamykany jednym przyciskiem,
  który odprowadza na ekran główny.

### G9 — Nie da się zmienić składu treningu w trakcie
- **Dla kogo/po co:** stanowisko zajęte, bark boli, zostało dziesięć minut — w
  praktyce trening przebudowuje się na miejscu. Dziś ćwiczenia można tylko
  przestawić kolejnością.
- **Dowód:** pasek akcji w `ScreenActiveWorkout.qml:101-143` ma trzy przyciski:
  timer, „Done" i zmianę kolejności. `ActiveWorkoutViewModel`
  (`src/ui/viewmodels/activeworkoutviewmodel.h:36-41`) ma operacje na seriach i
  `moveExercise`, ale żadnej na dodanie ani usunięcie ćwiczenia — mimo że ekran
  wyboru z katalogu (`ScreenExercisePicker.qml`) już istnieje i jest używany
  przez edytor.
- **Zakres:** M
- **Gotowe, gdy:** w aktywnym treningu da się dorzucić ćwiczenie z katalogu; po
  dodaniu pojawia się ono na liście, ma serie do odhaczenia, a po zakończeniu
  treningu widać je w historii.

### G10 — Filtr po mięśniu w katalogu ćwiczeń bez wejścia
- **Dla kogo/po co:** wybierając ćwiczenie na zastępstwo szuka się po partii
  („coś na biceps"), a nie po całej okolicy ciała.
- **Dowód:** `ExerciseCatalogViewModel` ma zapisywalny filtr `muscle`
  (`src/ui/viewmodels/exercisecatalogviewmodel.h:17,42`), a
  `ExerciseFilterPanel.qml:46-80` wystawia tylko `region`, `equipment` i `kind`.
  Ciąg „muscle" jako filtr nie pada w żadnym pliku QML.
- **Zakres:** S
- **Gotowe, gdy:** w panelu filtrów pickera jest wybór mięśnia; ustawienie go
  zmniejsza licznik dopasowanych ćwiczeń i zawęża listę, a „Clear filters"
  przywraca pełną liczbę.

### G11 — Kolejność serii w ćwiczeniu jest nie do zmiany
- **Dla kogo/po co:** rozgrzewkowa seria wpisana jako ostatnia zostaje na
  końcu; jedyne wyjście to usunąć i dodać na nowo, tracąc wpisane wartości.
- **Dowód:** `WorkoutEditorViewModel::moveSet` (`workouteditorviewmodel.h:57`)
  nie jest wywoływany z żadnego pliku QML. `SetRow.qml:171-197` oferuje przy
  serii wyłącznie duplikowanie i usunięcie.
- **Zakres:** S
- **Gotowe, gdy:** w edytorze treningu da się przesunąć serię w górę i w dół
  wewnątrz ćwiczenia; numeracja i wartości serii idą razem z nią, a zmiana
  przeżywa zapis.

### G12 — Notatka do ćwiczenia nie do wpisania
- **Dla kogo/po co:** „uchwyt szeroki", „ostatnio bolał nadgarstek", „taśma
  czerwona" — drobiazgi, bez których następny raz zaczyna się od zgadywania.
- **Dowód:** `WorkoutEditorViewModel::setExerciseNotes`
  (`workouteditorviewmodel.h:52`) nie ma wywołania w QML. Pole jest już
  wystawione do odczytu jako `ExerciseModel.description`
  (`src/ui/models/exercisemodel.h:14`) i wyświetlane w aktywnym treningu przez
  `ExerciseInfoPanel.qml:16`, więc czytanie działa, brakuje tylko pisania.
- **Zakres:** S
- **Gotowe, gdy:** w edytorze treningu da się wpisać notatkę do ćwiczenia; po
  zapisie i wystartowaniu treningu ta sama notatka jest widoczna w panelu
  informacji o ćwiczeniu.

### G15 — Nowe ćwiczenie zaczyna od zera kilogramów
- **Dla kogo/po co:** ćwiczenie wzięte z katalogu wchodzi do planu z „0 kg", a
  plan, w którym każda seria mówi zero, nie jest planem — użytkownik i tak musi
  wyklikać ciężar od nowa przy sztandze, choć aplikacja pamięta, ile podniósł
  ostatnim razem.
- **Dowód:** raport z iteracji 1 zauważył „0 kg" przy 8 powtórzeniach.
  Źródło: `WorkoutEditorViewModel::seedSet`
  (`workouteditorviewmodel.cpp:378-398`) ustawia powtórzenia, czas albo
  dystans, ale nigdy ciężaru. `addSet` na niepustym ćwiczeniu kopiuje już
  ostatnią serię wraz z ciężarem (`workouteditorviewmodel.cpp:180-184`), więc
  zero bierze się wyłącznie z pierwszej serii nowo dodanego ćwiczenia.
  Historia potrzebna do sensownej wartości jest już liczona i pokazywana w
  aktywnym treningu jako „Last time" (`WorkoutService::previousPerformances`,
  `src/application/workout/workoutservice.h:53`;
  `ActiveWorkoutExerciseItem.qml:221-230`).
- **Zakres:** L
- **Gotowe, gdy:** (do rozbicia — pierwszy plaster to podpowiedź w edytorze,
  ile użytkownik podniósł w tym ćwiczeniu ostatnim razem, zanim wartość zacznie
  być wstawiana automatycznie)

### G13 — Poprawienie zakończonego treningu
- **Dla kogo/po co:** telefon padł w połowie sesji albo seria została odhaczona
  przez pomyłkę — historia zostaje nieprawdziwa, a to ona karmi statystyki i
  prompt dla AI.
- **Dowód:** `WorkoutHistoryViewModel::saveWorkout`
  (`src/ui/viewmodels/workouthistoryviewmodel.h:34`) nie jest wywoływany z
  żadnego pliku QML. `WorkoutItem.qml:14-40` daje przy treningu z historii tylko
  kopiowanie do schowka i usunięcie — poprawka wymaga skasowania całej sesji.
- **Zakres:** L
- **Gotowe, gdy:** (do rozbicia — pierwszy plaster to poprawienie pojedynczej
  serii w zakończonym treningu, bez zmiany składu ćwiczeń)

### G14 — Szablon nie do przerobienia
- **Dla kogo/po co:** szablon to plan, do którego się wraca miesiącami; dziś po
  jednej zmianie w cyklu trzeba go zbudować od zera.
- **Dowód:** `ScreenWorkoutTemplates.qml:75-85` wystawia trzy akcje: użyj,
  duplikuj, usuń. `WorkoutTemplateViewModel` nie ma metody edycji ani zmiany
  nazwy, a jedyne wejście do zapisu szablonu to `saveAsTemplate` z edytora
  (`ScreenWorkoutEditor.qml:113`), które za każdym razem tworzy nowy.
- **Zakres:** L
- **Gotowe, gdy:** (do rozbicia — pierwszy plaster to zmiana nazwy istniejącego
  szablonu)

## Done

### G7 — Zapis w edytorze wygląda na martwy i niczego nie potwierdza (iteracja 2)
Warunek `enabled` przycisku Save zszedł z `valid && dirty` na samo `valid`, więc
poprawny trening rysuje się wypełnionym przyciskiem zamiast wyblakłego obrysu.
Obok przycisku doszedł komunikat z `validationErrors` — pierwszy konsument tej
właściwości w QML — a po udanym zapisie pojawia się potwierdzenie „Workout
saved". Sprawdzone w aplikacji na wszystkich trzech stanach.

### G1 — Edycja zaplanowanego treningu (iteracja 1)
Kafel zaplanowanego treningu dostał przycisk edycji, który otwiera edytor przez
istniejące `WorkoutEditorViewModel::edit(int)`. Sprawdzone w działającej
aplikacji: edytor wypełnia się nazwą i ćwiczeniami, a zapis po dodaniu serii
aktualizuje ten sam trening zamiast dokładać drugi obok.
