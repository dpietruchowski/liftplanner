# Product gaps

Backlog produktowy LiftPlannera. Każda pozycja opisuje, czego brakuje
człowiekowi, który trenuje z tą aplikacją w ręce, i wskazuje, gdzie to widać.

Kolejność sekcji „Open" to kolejność ważności dla użytkownika, nie trudności:
najpierw przepływy, które urywają się w połowie, potem zdolności, które
aplikacja ma, ale nigdy nie oferuje, potem rzeczy, po których traci się do niej
zaufanie, a dopiero na końcu nowe powierzchnie.

## Zmiana kierunku: dziennik, nie odhaczanie planu

Trzy prośby użytkownika z jednego wieczoru (G37, G38, G39) ciągną w tę samą
stronę, i to jest ważniejsze niż każda z nich osobno. Poprzedni wynik **przy
serii**, przycięcie zapisu **do tego, co zrobione**, i **trening bez planu**
mówią razem jedno: on prowadzi dziennik tego, co się wydarzyło, a nie odhacza
listę ułożoną wcześniej. Aplikacja jest zbudowana odwrotnie — plan najpierw,
sesja jako jego wykonanie — i stąd biorą się wszystkie trzy tarcia.

Nie unieważnia to planowania: import z AI zostaje sztandarową drogą wejścia i
komuś, kto ma cykl, plan nadal służy. Zmienia się to, **która droga jest
domyślna**, i przez to zmienia się waga reszty listy:

- **Rośnie** wszystko, co dotyczy zapisu po fakcie: poprawianie liczb w
  zakończonym treningu (G13b) i ciężar nieznany pokazywany jako „0 kg" (G18) —
  ten drugi wchodzi wprost na nową ścieżkę, bo w treningu bez planu ćwiczenie
  zaczyna się bez żadnej wartości.
- **Maleje** wszystko, co obsługuje układanie planu z wyprzedzeniem: historia
  ćwiczenia w edytorze (G35b) i przerabianie szablonów (G14). Piszę to o G35b
  wprost, bo sam ją dwie iteracje temu postawiłem na czele — pytanie „ile
  wpisać w plan" jest mniej ważne, jeśli plan przestaje być punktem wyjścia, a
  odpowiedź na nie i tak stoi teraz przy serii dzięki G37.

## Stan produktu po 25 iteracjach

Ocena product managera, nie sprawozdanie — spis zrobionej roboty jest w commitach
i w sekcji „Done". To jest punkt wyjścia dla następnej sesji.

### Co ta aplikacja dziś potrafi dla kogoś, kto z nią trenuje

Pętla treningowa jest domknięta od końca do końca i nie ma w niej miejsca, w
którym użytkownik zostaje bez wyjścia. Plan wchodzi importem od AI albo powstaje
ręcznie; trening startuje się z ekranu głównego albo z listy, i rusza ten, który
został wskazany; w trakcie odhacza się serie, odlicza przerwę ustawioną przy
ćwiczeniu, przestawia kolejność i dorzuca ćwiczenia, których nie było w planie —
z ciężarem z ostatniego razu. Sesję można zakończyć również wtedy, gdy nie
została dokończona: z choćby jednym ptaszkiem idzie do historii taka, jaka jest,
bez ptaszków wraca na listę zaplanowanych i nic z włożonej pracy nie ginie.
Po zakończeniu jest podsumowanie, a w historii — powtórzenie jednym przyciskiem.

Druga rzecz, ważniejsza od listy funkcji: **aplikacja przestała kłamać o tym, co
się zrobiło.** Jedna reguła rozstrzyga, co się liczy jako wykonane, i karmi
wszystko naraz — statystyki, „ostatnio", dziedziczenie ciężaru, historię i kopię
zapasową. Historia odróżnia serię zrobioną od opuszczonej, przerwany trening nie
znika po cichu przy zamianie, a eksport i ponowny import zachowują to, co
naprawdę zostało zrobione.

### Czego brakuje najbardziej — moim zdaniem, w tej kolejności

1. **Przeczytania własnego dziennika (G35).** Zapis wreszcie mówi prawdę, ale nie
   da się z niego odczytać przebiegu jednego ćwiczenia. Aplikacja odpowiada tylko
   „ostatnio"; „czy przysiad idzie w górę" wymaga rozwijania kafli historii jeden
   po drugim. To dziś największa rzecz, jakiej brakuje, i jedyna, która zamienia
   dziennik w narzędzie treningowe.
2. **Poprawienia liczb w zapisanym treningu (G13b).** Plaster pierwszy zamknął
   pomyłkę w odhaczeniu; ciężar i powtórzenia w zakończonej sesji nadal są tylko
   do czytania. Obejście istnieje — eksport, poprawka w tekście, import, i od G33
   przeżywa to wykonanie serii — więc pilność jest mniejsza niż przy plastrze
   pierwszym.
3. **Drobiazgów edytora, każdy S:** kolejność serii (G11), notatka do ćwiczenia
   (G12), filtr po mięśniu w katalogu (G10). Wszystkie trzy to zdolności, które
   siedzą gotowe w view modelach i czekają wyłącznie na wejście z ekranu.
4. **Rzeczy, które kłamią warunkowo:** „0 kg" tam, gdzie aplikacja po prostu nie
   zna ciężaru (G18), funty wybrane w profilu bez pokrycia w reszcie aplikacji
   (G29), pusty trening, który da się zapisać i wystartować (G17).

### Od czego zacząłbym dalej

Kolejność ustawiona po zmianie kierunku opisanej na górze: **G37 → G38 → G39**,
czyli trzy prośby użytkownika w tej kolejności, bo G38 ustala reguły kończenia
sesji, na których G39 się opiera (pusty trening porzucony bez niczego nie ma
planu, na który mógłby wrócić). Dopiero potem **G13b** i **G18** — obie obsługują
zapis po fakcie, czyli to, czym ta aplikacja właśnie się staje.

### Trzy wnioski metodyczne dla następnej sesji

- **Pułapka pustych flag `completed`.** Pięć spotkań w tej turze (G19, G23, G25,
  G31, G33). Każda pozycja dotykająca wykonania serii musi mieć w warunku
  akceptacji człon „a co z sesją, w której nie ma ani jednej flagi" — inaczej
  naprawa jednego kłamstwa produkuje drugie.
- **Obchód aplikacji własnymi oczami znajduje to, czego nie widać ani w kodzie,
  ani w celowanym zrzucie.** Przycięty przycisk startu (G28) był na trzech
  wcześniejszych zrzutach i nikt go nie zobaczył, bo każdy szukał czegoś innego.
  Zawyżona historia (G31) też wyszła dopiero z rozwinięcia kafla ręką.
- **Warunek akceptacji pisany negatywnie ratuje iterację.** Trzy razy człon „a to
  ma się **nie** zmienić" złapał regresję, zanim powstała.

## Open

### G40 — Okno, które kasuje dane, wygląda jak każde inne pytanie
- **Dla kogo/po co:** od G38 jest w aplikacji miejsce, w którym jedno kliknięcie
  bezpowrotnie usuwa serie. Treść okna mówi prawdę, ale układ prowadzi rękę ku
  skasowaniu: potwierdzenie jest dużym wypełnionym przyciskiem, wyjście —
  bladym obrysem obok, dokładnie jak w oknie pytającym „skopiować prompt?".
  Kto klika w biegu, między seriami, klika to, co wygląda na domyślne.
- **Dowód:** zauważone przez orkiestratora przy oglądaniu zrzutu z iteracji 28
  jako całości, nie w celowanym sprawdzeniu — czyli tą samą drogą co G28.
  `NotificationPopup` jest wspólny dla wszystkich okien i ma już
  `Theme.button.dangerSubtle`, używane przy usuwaniu serii, więc materiał
  istnieje; brakuje reguły, kiedy go użyć.
- **Dlaczego osobna pozycja, a nie dopisek przy okazji:** zmiana dotyka
  komponentu wspólnego dla **każdego** okna w aplikacji. To znaczy, że trzeba
  osobno sprawdzić, że pozostałe okna nie zmieniły się przy okazji — a takiej
  weryfikacji nie da się doczepić do cudzej pozycji bez rozmycia jej warunku.
- **Rozstrzygnięcie: tak, okna kasujące mają wyglądać inaczej — ale ważniejszy od
  koloru jest napis.** Kolor można przegapić: na słońcu, w ciemnym motywie, przy
  wadzie widzenia barw. „OK" nie mówi nic o skutku, a „Usuń 21 serii" mówi
  wszystko i działa niezależnie od tego, czy ktoś w ogóle zobaczy czerwień.
  Dlatego reguła brzmi: przycisk, który niszczy dane, nazywa to, co zniszczy, i
  nie jest przedstawiony jako domyślny wybór; bezpieczne wyjście jest co najmniej
  tak samo widoczne.
- **Reguła obejmuje wszystkie takie miejsca naraz**, nie jedno: kończenie treningu
  z przycięciem (G38), usunięcie zaplanowanego treningu, usunięcie treningu z
  historii, usunięcie serii, porzucenie zmian w edytorze. Zrobione tylko w jednym
  z nich, tworzy nową niespójność zamiast zamykać starą.
- **Zakres:** S
- **Gotowe, gdy:** okno kończące trening z nieodhaczonymi seriami ma przycisk
  nazywający usunięcie i odróżniający się od bezpiecznego wyjścia, a to wyjście
  nie jest słabiej widoczne niż potwierdzenie; to samo w oknach usuwania treningu
  zaplanowanego, treningu z historii, serii i porzucania zmian w edytorze;
  **zakończenie treningu, w którym odhaczono wszystko** — czyli takie, które
  niczego nie usuwa — wygląda jak zwykłe potwierdzenie, nie jak ostrzeżenie;
  okna, które o nic groźnego nie pytają (import, „prompt skopiowany", „workout
  saved", podsumowanie treningu), wyglądają dokładnie jak dotąd.

### G13b — Ciężar i powtórzenia w zakończonym treningu tylko do czytania
- **Dla kogo/po co:** drugi plaster po G13. Odhaczenie da się już poprawić, ale
  jeśli podniosłeś 65 kg zamiast zaplanowanych 60 i nie zmieniłeś tego przed
  ptaszkiem, historia zostaje z sześćdziesiątką — i tę sześćdziesiątkę dziedziczy
  następny trening, statystyki i prompt dla AI.
- **Dowód:** `WorkoutHistoryViewModel::saveWorkout` przyjmuje cały trening, więc
  droga zapisu istnieje i po G13 jest już używana; w rozwiniętym kaflu historii
  klikalne jest wyłącznie wykonanie serii, sama liczba nie.
- **Waga:** niższa niż plaster pierwszy, bo obejście istnieje i od G33 jest
  bezpieczne: eksport do schowka, poprawka w tekście, import z powrotem —
  wykonanie serii przeżywa tę podróż.
- **Zakres:** M
- **Gotowe, gdy:** w rozwiniętym kaflu treningu z historii da się zmienić ciężar
  i powtórzenia pojedynczej serii; zmiana przeżywa restart; objętość i „ostatnio"
  na ekranie głównym liczą się po zmianie z nowej wartości; wykonanie serii nie
  zmienia się przy okazji edycji liczby.

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

### G35b — Układając plan, nie widzisz, co robiłeś
- **Dla kogo/po co:** drugi plaster po G35. Historię jednego ćwiczenia widać już
  w trakcie treningu, czyli wtedy, gdy na zmianę planu jest za późno. Pytanie
  „ile wpisać" pada wcześniej — przy układaniu treningu na przyszły tydzień, w
  edytorze. Tam użytkownik nadal jest ślepy: widzi liczby, które ktoś (AI,
  powtórzenie, on sam miesiąc temu) już wpisał, i nie ma jak sprawdzić, czy mają
  sens wobec tego, co naprawdę dźwigał.
- **Dowód:** obejrzałem edytor w działającej aplikacji
  (`tmp/loop/pm16-editor.png`, po G6 doszedł wiersz przerwy): przy ćwiczeniu są
  strzałki kolejności, kosz, przerwa, serie i „Add set" — ani słowa o historii.
  Jedyne dzisiejsze wsparcie to zasiew z G15, i to wyłącznie w chwili dodawania
  ćwiczenia z katalogu; trening z importu albo z powtórzenia nie zasiewa niczego.
  Zapytanie, którego brakowało, **już istnieje** — `exerciseSessions(exercise,
  limit)` (`workoutservice.h`, dołożone w iteracji 26) przyjmuje dowolne
  ćwiczenie i dopasowuje je przez `sameExercise`, więc pozostaje wejście z ekranu.
- **Zakres:** M (wejście w edytorze; wejście z katalogu przy wybieraniu ćwiczenia
  to osobny, późniejszy plaster — tam pada inne pytanie: „które", nie „ile")
- **Gotowe, gdy:** w edytorze treningu przy ćwiczeniu da się otworzyć jego
  ostatnie sesje i widać tę samą listę co w trakcie treningu — data i co w niej
  zrobiono, od najnowszej; ćwiczenie bez historii mówi to wprost; ćwiczenie
  pochodzące z importu od AI, czyli niepowiązane z katalogiem, też dostaje swoje
  sesje, a nie pustą listę; otwarcie i zamknięcie tego widoku niczego w treningu
  nie zmienia — po wyjściu z niego zapis zachowuje się tak samo jak przed
  otwarciem, a edytor nie zaczyna twierdzić, że ma niezapisane zmiany.

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

### Dwie linie nazwy na bębnie zabierają listę ćwiczeń (iteracja 16)
Kompromis z G30, przyjęty świadomie. W momencie naciskania startu liczy się to,
**który** trening ruszy, a nie z czego się składa; podgląd ćwiczeń zostaje w
wierszach niezaznaczonych, na kaflu listy zaplanowanych i po rozwinięciu kafla.
Przy krótkiej nazwie widać jedno i drugie.

### Nazwa 46-znakowa nadal ucina się na bębnie (iteracja 16)
Zachowanie zamierzone: wielokropek jako ostatnia deska ratunku powyżej dwóch
linii, żeby układ ekranu głównego się nie rozjechał. Pełny tytuł jest o jedno
przejście stąd — ten sam trening na kaflu listy zaplanowanych mieści się w
całości. Nikt nie nazywa dnia planu zdaniem na 46 znaków częściej niż raz.

### Pusta przestrzeń nad bębnem przy dwóch pozycjach (iteracja 15)
Zgłoszone dla porządku razem z G28. Treść jest dosunięta do dołu i nic się nie
psuje — użytkownik nie traci ani informacji, ani akcji. Rozstrzygnięcie odstępów
w pustym stanie to robota na wtedy, gdy ekran główny dostanie nową zawartość, nie
osobna iteracja.

## Scalone

Identyfikatory nie wracają do obiegu, więc pozycje wchłonięte przez inne
zostają tutaj.

### G36 — Pusty bęben mówi „---" → G39
Pusty stan ekranu głównego to dokładnie miejsce, w którym ma stanąć wejście do
treningu bez planu, więc komunikat i wejście robi się raz, nie dwa razy. Warunek
G39 obejmuje jedno i drugie.

### G20 — Zaimportowana historia przychodzi bez śladu wykonania → G31 (iteracja 18)
Warunek akceptacji tej pozycji brzmiał: serie zaimportowanej historii mają być
widoczne jako zrobione w podglądzie treningu z historii. Dokładnie to dowiozło
G31 — trening „Base Strength" z importu pokazuje dwadzieścia żetonów jako
zrobione, mimo zera flag w bazie, bo rozstrzyga o tym wspólna reguła z
`performedsets.h`, a nie surowa flaga. W danych flag nadal nie ma i nie musi być:
człowiek nie widzi z tego powodu żadnej złej liczby.

### G24 — Kafelki na ekranie głównym mówią dwoma stylami → G8 (iteracja 10)
Etykiety kafelków w trzech konwencjach naraz („Bench Press", „volume", „AGE").
Osobna iteracja na samą wielkość liter byłaby marnotrawstwem, a G8 i tak dokłada
kafelki podsumowania — więc warunek spójności etykiet stał się częścią G8.

## Done

### G39 — Nie da się zacząć treningu bez planu (iteracja 29, wchłania G36)
„Bez planu" stoi jako pozycja na bębnie, więc trening zaczyna się tym samym
przyciskiem co każdy inny i żaden drugi nie powstał. Pusta sesja dostaje nazwę z
dnia — bęben nie mówi już „---" — a ekran bez ćwiczeń prowadzi jednym zdaniem i
jednym przyciskiem. Porzucona pusta sesja znika z bazy, ale dopiero po zdaniu,
które to zapowiada, i tym samym oknem co G38. Sprawdzone na bazie: anulowanie nie
ruszyło ani wiersza, potwierdzenie zabrało sam trening i nic poza nim.
**Otwarte po tej pozycji:** dwie sesje bez planu z tego samego dnia mają
identyczną nazwę i na liście nie da się ich rozróżnić — do rozstrzygnięcia razem
z nazywaniem sesji z G14.

### G38 — Kończąc trening, nie wiadomo, że coś zostaje niezrobione (iteracja 28)
Okno kończące trening mówi, ile serii znika i że bezpowrotnie, a po potwierdzeniu
w historii zostaje wyłącznie to, co zrobione — ćwiczenia bez ani jednej odhaczonej
serii wypadają w całości. Sprawdzone na bazie: anulowanie nie ruszyło ani jednego
wiersza, potwierdzenie zabrało dokładnie te 21 serii i 6 ćwiczeń, które okno
obiecało. Dziewiąte spotkanie z regułą pustych flag, tym razem groźne w drugą
stronę: sesja bez ani jednej flagi jest wykonana w całości, więc przycięcie jej
skasowałoby cały trening. Kasowanie zdarza się wyłącznie tu — odznaczenie serii
w historii (G13) nadal ją zachowuje, a starsze wpisy mają przekreślenia jak dotąd.

### G37 — Poprzedni wynik przy każdej serii z osobna (iteracja 27)
Wiersz serii pokazuje wynik z tej samej pozycji poprzedniej sesji, a nagłówek
ćwiczenia oddał liczby i został przy dacie. Ósme spotkanie pętli z regułą pustych
flag, po raz pierwszy bez dopisywania czegokolwiek do `performedsets.h`. Dwa
zawroty, oba o szerokość: kolumna podpowiedzi ucinała `10xBW+22.5kg`, a jej
poszerzenie zabrało miejsce napisom, które wcześniej się mieściły. Rozwiązało to
dopiero skrócenie `BW + 22.5 kg` do `+22.5 kg`, bo bodyweight wynika z samego
ćwiczenia. Pilnuje tego `SetRowFitTest`, który mierzy obie kolumny w obu krojach
wiersza przeciwko liczbom odczytanym z `Theme.qml`.

### G13 — Poprawienie zakończonego treningu, plaster 1: jedna seria (iteracja 23)
Żeton serii w rozwiniętym kaflu historii jest klikalny i przełącza wykonanie;
zmiana przeżywa restart, a kafel nie zwija się przy zapisie. Sesja bez ani jednej
flagi najpierw ostrzega, że pierwszy ptaszek odbiera jej amnestię i pozostałe
serie staną się opuszczone — „Cancel" nie rusza ani ekranu, ani bazy. Szóste
spotkanie pętli z pułapką pustych flag i pierwsze, w którym nie dało się jej
uniknąć: sesja bez flag musi ją stracić, więc rzecz była w tym, żeby nie
zaskoczyć nią człowieka.

### G32 — Aplikacja nie umie odmieniać tego, co policzyła (iteracja 22)
Jedno miejsce odmienia policzone rzeczy, więc popup kończący trening i kafel
szablonu przestały mówić „1 exercises" i „1 of 22 sets are". Zlepione z trzech
osobnych wystąpień właśnie po to, żeby czwarty licznik nie odtworzył problemu.

### G33 — Kopia zapasowa historii wracała jako trening zrobiony w całości (iteracja 21)
Eksport i ponowny import zachowują teraz to, które serie były zrobione — wcześniej
kompaktowy zapis gubił wykonanie, a amnestia dla sesji bez flag zamieniała „1 z 22"
w „22 z 22". Stary format nadal się importuje, plan wchodzi jako niezrobiony.

### G26 — Nieudany import historii milczał (iteracja 20)
Śmieć w schowku, pusty schowek, zły kształt JSON-a, element bez nazwy i błąd
zapisu mają osobne komunikaty; udany import nadal nic nie mówi.

### G9 — Nie dało się dorzucić ćwiczenia w trakcie treningu (iteracja 19)
Pasek akcji aktywnego treningu otwiera ten sam picker co edytor. Dorzucone
ćwiczenie ląduje na końcu z serią do odhaczenia, wchodzi z ciężarem i rodzajem z
ostatniego razu (zasiew przechodzi przez amnestię, więc widzi też historię z
importu) i trafia do historii oznaczone dokładnie tak, jak zostało odhaczone.
Przeżywa też powrót sesji na listę zaplanowanych.

### G31 — Historia pokazywała serię opuszczoną tak samo jak zrobioną (iteracja 18)
Żeton serii mówi, czy seria została zrobiona, i rozstrzyga o tym wspólna reguła z
`performedsets.h`, nie surowa flaga — dzięki czemu sesja bez ani jednej flagi
(import) nadal pokazuje się jako zrobiona, a kafel zaplanowany nie oznacza
wszystkiego jako opuszczone.

### G6 — Czas przerwy był niewidoczny i nie do ustawienia (iteracja 17)
W edytorze przy każdym ćwiczeniu jest wiersz „REST − wartość +", wartość czyta się
z prawdziwych danych, przeżywa zapis, a timer po odhaczeniu serii odlicza właśnie
od niej. Nadpisanie przy pojedynczej serii świadomie zostało poza zakresem.

### G30 — Na bębnie nie dało się przeczytać, co się wystartuje (iteracja 16)
Zaznaczony wiersz zawija nazwę na dwie linie, niezaznaczone ściskają ją zamiast
elidować. Wysokość ekranu głównego z G28 nie ucierpiała.

### G28 — Ekran główny nie mieścił się i tnął własny przycisk startu (iteracja 15)
Kolumna treści dostała twardy limit wysokości, nadmiar pochłania bęben. Nagłówek z
datą i przycisk startu z pełnym napisem mieszczą się na 360 × 640.

### G16 — Nazwa treningu na kaflu przegrywała z przyciskami (iteracja 14)
Edycja i usunięcie zeszły do części rozwijanej, w nagłówku został sam start, a
tytuł zawija się zamiast elidować.

### G27 — Zamiana trwającego treningu kasowała go bez ostrzeżenia (iteracja 13)
Przerwana sesja z ptaszkiem idzie do historii, bez ptaszków wraca na listę
zaplanowanych, a potwierdzenie mówi z góry, co się stanie. Wcześniej wiersz
znikał z bazy całkiem.

### G4 — „Start workout" ignorował wybór na bębnie (iteracja 12)
Startuje trening zaznaczony na bębnie. Pozycja, której nie da się wystartować,
tłumaczy dlaczego i odsyła do powtórzenia z historii, zamiast ruszać cudzy trening.

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
