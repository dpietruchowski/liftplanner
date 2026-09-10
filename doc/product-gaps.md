# Product gaps

Backlog produktowy LiftPlannera. Każda pozycja opisuje, czego brakuje
człowiekowi, który trenuje z tą aplikacją w ręce, i wskazuje, gdzie to widać.

Kolejność sekcji „Open" to kolejność ważności dla użytkownika, nie trudności:
najpierw przepływy, które urywają się w połowie, potem zdolności, które
aplikacja ma, ale nigdy nie oferuje, potem rzeczy, po których traci się do niej
zaufanie, a dopiero na końcu nowe powierzchnie.

## Open

### G2 — Wybór dnia zaplanowanego treningu
- **Dla kogo/po co:** manualnie utworzony trening zawsze ląduje na „teraz", więc
  tygodnia nie da się rozłożyć na poniedziałek, środę i piątek z wyprzedzeniem.
- **Waga po sprawdzeniu:** mniejsza, niż pisałem wcześniej. Import z AI, czyli
  sztandarowa droga do tej aplikacji, **przenosi daty** — `planned_time` jest i
  w zapisie, i w odczycie (`workoutjson.cpp:340-341`). Dziura dotyczy więc
  wyłącznie ścieżki ręcznej i szablonów, nie całego planowania.
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

### G26 — Nieudany import historii milczy
- **Dla kogo/po co:** wklejasz do schowka JSON od AI, klikasz import historii,
  potwierdzasz — i nie dzieje się absolutnie nic. Nie wiadomo, czy plan był zły,
  czy schowek pusty, czy aplikacja zamarła. Ta sama akcja obok, przy „Planned",
  potrafi powiedzieć, co poszło nie tak, więc użytkownik dostaje dwa różne
  zachowania na dwóch sąsiednich przyciskach tego samego ekranu.
- **Dowód:** `MainView.qml:148-180` podpina komunikaty o błędach do trzech view
  modeli: `PlannedWorkoutViewModel`, `WorkoutEditorViewModel`,
  `WorkoutTemplateViewModel`. `WorkoutHistoryViewModel` na tej liście nie ma, a
  zgłasza błędy w sześciu miejscach, m.in. „Import failed: invalid JSON"
  (`workouthistoryviewmodel.cpp:218`) i „Clipboard is empty" (`:242`) — to
  dokładnie ścieżka przycisku `importHistoryButton`
  (`ScreenWorkouts.qml:164-173`).
- **Zawężone po iteracji 12:** druga połowa tej pozycji („martwy przycisk startu")
  odpadła — G4 dołożył `cannotStartPopup`, który mówi, czemu minionego treningu
  nie da się wystartować, i odsyła do powtórzenia z historii. Zostaje sam import.
- **Zakres:** S
- **Gotowe, gdy:** import historii ze śmieciem w schowku (albo z pustym
  schowkiem) pokazuje komunikat mówiący, co się nie udało, a poprawny import
  nadal wchodzi bez ostrzeżeń.

### G29 — Wybór funtów w profilu kończy się na profilu
- **Dla kogo/po co:** ustawiasz „Imperial (lb)", a cała reszta aplikacji dalej
  mówi „kg" — przy każdej serii, przy rekordach na ekranie głównym, w objętości.
  Aplikacja obiecuje jednostkę, której nie dotrzymuje, a przy ciężarach to
  różnica ponad dwukrotna.
- **Dowód:** `ScreenProfile.qml:316-322` daje wybór `unitSystem` (metric /
  imperial) i kafelek wagi ciała rzeczywiście przechodzi na `bodyweightUnit`
  (`:176-180`). Poza tym ekranem nikt tej właściwości nie czyta: „kg" jest wpisane
  na stałe w `setmodel.cpp:62` (opis serii), `workouttext.cpp:37` (objętość) i
  `ScreenHome.qml:80` (kafelek rekordu). Ciąg „unitSystem" nie pada w żadnym
  innym pliku QML.
- **Uwaga:** pozycja niżej niż reszta, bo wymaga świadomego przestawienia
  przełącznika, którego domyślny użytkownik nie tknie. Ale kto go tknie, dostaje
  dwa ekrany mówiące o tej samej wadze co innego.
- **Zakres:** M
- **Gotowe, gdy:** po przestawieniu profilu na „Imperial (lb)" jednostka przy
  serii w aktywnym treningu, przy kafelku rekordu i przy objętości na ekranie
  głównym jest ta sama co w profilu; powrót na „Metric (kg)" przywraca stan
  sprzed zmiany.

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
- **Uwaga po G19 i G21:** pilność spadła. Odkąd sesja bez ani jednego ptaszka
  liczy się w całości, zaimportowana historia znów zasila „Last time",
  dziedziczenie ciężaru i statystyki. Zostaje niezgodność tego, co użytkownik
  widzi w podglądzie treningu (nic nieodhaczone), z tym, co aplikacja z tego
  wnioskuje — warto naprawić, ale to już nie blokuje niczego.

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

## Odrzucone

Zgłoszenia sprawdzone i świadomie niebrane na backlog — żeby nie wracały.

### Trening wrócony do planu zachowuje stary `started_time` (iteracja 13)
Sprawdzone: nigdzie nie kłamie. Kafel zaplanowanego treningu pokazuje
`plannedTime`, a bęben na ekranie głównym stawia taki trening po stronie planów.
Człowiek nie widzi z tego powodu ani złej daty, ani złej liczby, więc to sprawa
porządku w bazie, nie produktu. Wraca na listę dopiero wtedy, gdy pojawi się
ekran, na którym ta data będzie widoczna.

### Pusta przestrzeń nad bębnem przy dwóch pozycjach (iteracja 15)
Zgłoszone dla porządku razem z G28. Treść jest dosunięta do dołu i nic się nie
psuje — użytkownik nie traci ani informacji, ani akcji. Rozstrzygnięcie odstępów
w pustym stanie to robota na wtedy, gdy ekran główny dostanie nową zawartość, nie
osobna iteracja.

## Scalone

Identyfikatory nie wracają do obiegu, więc pozycje wchłonięte przez inne
zostają tutaj.

### G24 — Kafelki na ekranie głównym mówią dwoma stylami → G8 (iteracja 10)
Etykiety kafelków w trzech konwencjach naraz („Bench Press", „volume", „AGE").
Osobna iteracja na samą wielkość liter byłaby marnotrawstwem, a G8 i tak dokłada
kafelki podsumowania — więc warunek spójności etykiet stał się częścią G8.

## Done

### G25 — Nie da się powtórzyć treningu, który się już zrobiło (iteracja 11)
Rozwinięty kafel w historii ma akcję powtórzenia: tworzy nowy zaplanowany trening
na dziś z tymi samymi ćwiczeniami i seriami, bez edytora i bez AI. Historia
przestała być ślepym zaułkiem. Kopia idzie przez `SetPrescription`, więc brak
odhaczeń wynika z typu, a nie z ręcznego resetu; ciężary są te z powtarzanej
sesji, a nie z zasiewu, żeby akcja nie skłamała o tym, co użytkownik wskazał.
Sprawdzone w aplikacji na przypadku czasowym i z ciężarami: oryginał nietknięty,
kopia bez ptaszków, lista odświeża się bez restartu.

### G8 — Brak podsumowania po zakończonym treningu (iteracja 10, domyka G24)
Po zakończeniu sesji otwiera się podsumowanie z czasem, odhaczonymi seriami i
objętością tej sesji, zamykane jednym przyciskiem prowadzącym na ekran główny.
Liczby idą przez tę samą `countsAsPerformed` co ekran główny, więc sesja urwana
pokazuje to, co zrobiono — sprawdzone: jedna odhaczona seria z dwóch to „1/2" i
200 kg, nie 400. Porzucenie bez ptaszków nie pokazuje podsumowania. Etykiety
kafelków ujednolicone do wersalików, wymuszonych w `StatTile`, więc kolejna
konwencja nie ma jak powstać. Przy okazji `objectName` dla kafli historii i
profilu.

### G23 — Nie da się zakończyć treningu, którego się nie dokończyło (iteracja 9)
Pasek akcji aktywnego treningu dostał `finishWorkoutButton` widoczny przez całą
sesję. Odhaczona co najmniej jedna seria → trening idzie do historii z flagami
takimi, jakie są, a serie nietknięte zostają nietknięte. Zero odhaczonych serii →
sesja nie jest zapisywana jako trening, tylko wraca na listę zaplanowanych. To
drugie rozstrzyga zderzenie z regułą z G19 bez ruszania samej reguły: nowa droga
po prostu nie potrafi wyprodukować sesji zakończonej bez ani jednego ptaszka.
Sprawdzone w aplikacji na obu gałęziach, łącznie z natychmiastowym odświeżeniem
listy zaplanowanych i historii.

### G22 — Objętość treningu jest liczona i wyrzucana (iteracja 8)
Ekran główny dostał trzeci kafelek statystyk z `TrainingTotals.totalWeight`,
formatowany przez nowe `WorkoutText::formatVolume` z grupowaniem tysięcy. Zero
zmian w QML — delegat już budował `objectName` z etykiety. Sprawdzone w
aplikacji: `totalsTile_volume` = „2 755 kg", pasek statystyk sztangisty przestał
być pusty. Domyka też obserwowalną część G21.

### G21 — Trzy różne reguły „co się liczy jako zrobione" (iteracja 7)
`countsAsPerformed` w `performedsets.h` jest teraz jedynym rozstrzygnięciem;
`previousPerformances`, `recentTotals` i `topExercises` tylko je karmią, każde
swoim kształtem danych. Doszedł test równoważności obu kafelków ekranu głównego.
**Zastrzeżenie:** obserwowalnej części warunku akceptacji nie dało się pokazać —
kafelka objętości aplikacja w ogóle nie ma, a danych czasowych i dystansowych w
historii nie ma wcale. Przyjęte na podstawie testów i braku regresji; szczegóły
w raporcie z iteracji 7.

### G19 — Zakończony trening nie liczy się jako wykonany (iteracja 6)
Reguła: w zakończonym treningu seria liczy się, jeśli ma flagę; jeśli żadna seria
w całej sesji jej nie ma, flaga nic nie niesie i liczą się wszystkie serie z
zawartością. Granulacja per sesja, nie per ćwiczenie, żeby nie wskrzeszać
ćwiczeń świadomie odpuszczonych. Polityka wydzielona do `domain/workout/performedsets.h`.
Sprawdzone w aplikacji: „Last time" dla Back Squata pokazuje wreszcie sesję z
8 czerwca (5x20, 5x30, 3x5x40 kg), edytor zasiewa 40 kg, a dla Bench Pressa
nowsza odhaczona sesja nadal wygrywa ze starszą nieodhaczoną.
Sprostowanie do mojego opisu tej pozycji: twierdziłem, że `recentTotals` nie
filtruje po fladze — filtruje (`workoutservice.cpp:219`), przeoczyłem to, bo
szukałem `completed()` z nawiasami, a tam jest pole `row.completed`. Zakres
naprawy był przez to za wąski; reszta poszła do G21.

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
