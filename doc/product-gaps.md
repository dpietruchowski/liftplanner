# Product gaps

Backlog produktowy LiftPlannera. Każda pozycja opisuje, czego brakuje
użytkownikowi na siłowni, i wskazuje miejsce w kodzie, z którego to wynika.
Kolejność sekcji „Open" to kolejność ważności.

## Open

### G21 — Statystyki na ekranie głównym mają tę samą ślepotę co „Last time"
- **Dla kogo/po co:** po naprawie G19 „Last time" pokazuje sesję z czerwca, ale
  kafelki objętości i czasu na ekranie głównym dalej liczą tę samą sesję jako
  zerową. Użytkownik widzi teraz dwie sprzeczne odpowiedzi na to samo pytanie.
- **Dowód:** znalezione przez developera przy G19 i sprostowane w opisie:
  `recentTotalsCore` ma `if (!row.hasSet || !row.completed) continue`
  (`workoutservice.cpp:219`), czyli filtruje po fladze dokładnie tak, jak robiło
  to `previousPerformancesCore` przed naprawą. `topExercises` nie filtruje, więc
  lista najczęstszych ćwiczeń była i jest poprawna — stąd wrażenie, że część
  ekranu głównego działa.
- **Zakres:** M — nie da się przenieść reguły z G19 wprost, bo `recentTotals`
  chodzi po płaskich `HistorySetRow`, a nie po agregacie `Workout`, więc
  „czy w tej sesji ktokolwiek odhaczał" trzeba tam ustalić inaczej.
- **Gotowe, gdy:** dla historii, w której żadna seria nie ma flagi, kafelki
  objętości i czasu na ekranie głównym pokazują wartości z tych treningów, a nie
  zera; historia z częściowym odhaczeniem nadal liczy wyłącznie odhaczone serie.

### G20 — Zaimportowana historia przychodzi bez śladu wykonania
- **Dla kogo/po co:** import planu i historii z AI to sztandarowa droga do
  aplikacji, a wszystko, co tą drogą wchodzi, jest oznaczone jako niezrobione.
  To źródło zer, na które natrafił raport, i dopóki działa, każdy kolejny import
  dokłada danych, które dla aplikacji nie istnieją.
- **Dowód:** `parseSets` (`workoutjson.cpp:229` i dalej) buduje serie z
  kompaktowego zapisu (`"5x60kg,5x75kg"`) i nigdy nie dotyka `completed`;
  format kompaktowy, czyli ten, który produkuje prompt dla AI, w ogóle nie ma
  takiego pola. Pełny JSON ustawia flagę tylko wtedy, gdy klucz jest obecny
  (`workoutjson.cpp:120-121`). Trening zaimportowany ze statusem „ended" ląduje
  więc w bazie z każdą serią na zero.
- **Zakres:** M
- **Gotowe, gdy:** trening zaimportowany jako historia (status zakończony) ma po
  imporcie serie oznaczone jako wykonane, widoczne jako odhaczone w podglądzie
  treningu z historii; import planu na przyszłość nadal wchodzi jako niezrobiony.

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

### G16 — Nazwa treningu na kaflu przegrywa z przyciskami
- **Dla kogo/po co:** nazwa jest jedyną rzeczą, po której odróżnia się „Push A"
  od „Push B" na liście. Dziś ucina się po kilkunastu znakach, a kafel oddaje
  większość szerokości przyciskom, z których dwa (edycja, usunięcie) używane są
  raz na kilka tygodni.
- **Dowód:** raport z iteracji 3 (`tmp/loop/test-report.md`, „Obserwacje",
  zrzut `tmp/loop/g3-confirm.png`): „Naming che..." przy szerokości, na której
  zmieściłoby się dużo więcej. Nagłówek kafla (`WorkoutCard.qml:46-77`) to
  przycisk rozwijania + tytuł z `Layout.fillWidth` + rząd akcji; do
  `PlannedWorkoutItem.qml` doszły w iteracjach 1 i 3 przyciski edycji i
  usunięcia, więc rząd akcji urósł z jednego przycisku do trzech i tytuł dostaje
  resztę przy oknie 360 px (`Main.qml:9`). Wzorzec, który to rozwiązuje, jest już
  w repo: `WorkoutItem.qml:14-40` trzyma rzadkie i groźne akcje w
  `expandedActions`, czyli pokazuje je dopiero po rozwinięciu kafla.
- **Zakres:** S
- **Gotowe, gdy:** przy oknie 360 px kafel zaplanowanego treningu pokazuje pełną
  nazwę długości typowej dla treningu (np. „Naming check upper A") bez wielokropka,
  a edycja i usunięcie pozostają osiągalne.

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

### G17 — Pusty trening da się zapisać i wystartować
- **Dla kogo/po co:** trening bez ani jednego ćwiczenia trafia na listę
  zaplanowanych i można go wystartować. Użytkownik ląduje wtedy na ekranie
  aktywnego treningu, na którym nie ma czego odhaczyć i z którego nie widać
  wyjścia — wygląda to jak zepsuta aplikacja, a nie jak własna pomyłka.
- **Dowód:** raport z iteracji 3 („Obserwacje"): trening z nazwą, ale bez
  ćwiczeń, wciąż jest zapisywalny. `Workout::validationErrors`
  (`src/domain/workout/workout.cpp:75-89`) sprawdza tylko nazwę i deleguje
  resztę do ćwiczeń — przy pustej liście pętla nie wykonuje się ani razu, więc
  trening jest „valid" i przycisk „Save" w edytorze aktywny
  (`ScreenWorkoutEditor.qml:59`). Na ekranie aktywnego treningu przycisk „Done"
  jest wtedy wyłączony, bo nie ma `currentSet` (`ScreenActiveWorkout.qml:119`).
- **Zakres:** S
- **Gotowe, gdy:** w edytorze trening bez ćwiczeń nie da się zapisać, a przy
  zablokowanym przycisku widać, że brakuje ćwiczeń; trening z co najmniej jednym
  ćwiczeniem zapisuje się jak dotąd.

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

### G18 — Seria bez ciężaru wygląda jak seria z zerowym ciężarem
- **Dla kogo/po co:** reszta po G15 — ćwiczenie robione pierwszy raz w życiu
  albo plan z AI, który nie podał obciążenia. „0 kg" to wtedy nieprawda podana
  z pewnością siebie; użytkownik nie wie, czy aplikacja mu każe wziąć pustą
  sztangę, czy po prostu nie wie.
- **Dowód:** ta sama ścieżka co w G15 — `SetRow.qml` pokazuje `secondaryText`
  bez rozróżnienia „zero" od „nieustawione", a `Set` trzyma zwykły `double`
  bez stanu „brak wartości".
- **Zakres:** M
- **Gotowe, gdy:** seria, dla której nie ma ani historii, ani wartości z planu,
  pokazuje w aktywnym treningu i w edytorze znak braku wartości zamiast „0 kg",
  a po ustawieniu ciężaru zachowuje się jak każda inna.

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

### G19 — Zakończony trening nie liczy się jako wykonany (iteracja 6)
Reguła: w zakończonym treningu seria liczy się, jeśli ma flagę; jeśli żadna seria
w całej sesji jej nie ma, flaga nic nie niesie i liczą się wszystkie serie z
zawartością. Granulacja per sesja, nie per ćwiczenie, żeby nie wskrzeszać
ćwiczeń świadomie odpuszczonych. Polityka wydzielona do `domain/workout/performedsets.h`.
Sprawdzone w aplikacji: „Last time" dla Back Squata pokazuje wreszcie sesję z
8 czerwca (5x20, 5x30, 3x5x40 kg), edytor zasiewa 40 kg, a dla Bench Pressa
nowsza odhaczona sesja nadal wygrywa ze starszą nieodhaczoną.

### G15 — Nowe ćwiczenie nie dziedziczy ciężaru z poprzedniego razu (iteracja 5)
`WorkoutService::lastPerformance` deleguje do istniejącego
`previousPerformancesCore`, a edytor po dodaniu ćwiczenia z katalogu przepisuje
ciężar i powtórzenia z ostatniej ukończonej serii. Sprawdzone w aplikacji na
obu gałęziach: ćwiczenie z historią wchodzi z 2,5 kg i tę wartość ma po
wystartowaniu treningu, ćwiczenie bez historii zostaje przy zasiewie z katalogu.

### G5 — Edytor serii w aktywnym treningu nie daje o sobie znać (iteracja 4)
Wiersz serii dostał szewron rozwijający edytor jednym kliknięciem; przytrzymanie
zostało jako skrót. Edytor treningu wyłącza tę ścieżkę przez `expandable: false`,
więc tam nic się nie zmieniło. Sprawdzone w aplikacji: rozwijanie, zwijanie,
kropka ukończenia dalej odhacza serię zamiast rozwijać.

### G3 — Usunięcie zaplanowanego treningu (iteracja 3)
`PlannedWorkoutViewModel` dostał `deleteWorkout(WorkoutModel*)`, a kafel
zaplanowanego treningu czerwony przycisk z potwierdzeniem, wzorowany na akcji
usuwania w historii. Sprawdzone w aplikacji łącznie z restartem: usunięty
trening nie wraca.

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
