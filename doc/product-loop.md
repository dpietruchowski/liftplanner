# Pętla produktowa

**Po kompakcji kontekstu przeczytaj ten plik jako pierwszy, potem
[product-gaps.md](product-gaps.md), i wracaj do pętli od stanu opisanego na
dole.** Nie pytaj użytkownika o zgodę — praca leci bez nadzoru aż do progu
limitu albo do wyczerpania backlogu.

## Obsada

| rola | kto | co robi |
|---|---|---|
| product manager | agent `product-manager` | decyduje, czego brakuje; prowadzi `doc/product-gaps.md`; wybiera jedną pozycję |
| developer | agent `developer` | implementuje tę jedną pozycję, pisze testy jednostkowe, buduje |
| tester + commiter | orkiestrator (główna sesja) | prowadzi apkę, ogląda zrzuty, commituje |

## Obrót pętli

1. **Limit.** `bash tmp/probe_usage.sh` (próg 85%). Kod wyjścia 1 albo `STOP` →
   przerywam natychmiast, nic nie zaczynam, dopisuję stan na dole tego pliku.
2. **Product manager.** Dostaje raport z poprzedniej iteracji
   (`tmp/loop/test-report.md`). Oddaje jedną pozycję i warunek „gotowe, gdy".
3. **Developer.** Dostaje id pozycji i ten warunek. Oddaje listę plików,
   `objectName` do klikania i wynik testów. Nie commituje.
4. **Ja — test.** Buduję, `ctest`, a przy zmianach w QML dodatkowo
   `cmake --build build-desktop -j 4 --target liftplanner_qml_qmllint`. Potem sesja UI: przechodzę ścieżkę
   użytkownika, sprawdzam warunek akceptacji, robię zrzut, czytam log aplikacji.
   Wynik zapisuję do `tmp/loop/test-report.md`.
   Nie działa → wracam do developera z konkretem, maksymalnie dwa razy; za
   trzecim odkładam pozycję i notuję dlaczego.
5. **Ja — commit.** `git commit -- <ścieżki>`, tryb rozkazujący, bez trailerów.
   Ścieżki biorę z raportu developera i z `git status --porcelain`.
   Odhaczam pozycję w `product-gaps.md` i aktualizuję stan na dole.
6. Wracam do 1.

**Zielony build i zielony `ctest` nie są dowodem, że aplikacja działa.**
Sprawdzone na G5 (iteracja 4): `SetRow.qml` używał `ToolTip` bez
`import QtQuick.Controls`, build przeszedł bez ostrzeżeń, 704 testy na zielono,
a aplikacja nie wstawała — cały ekran aktywnego treningu niedostępny.
`qmlcachegen` sprawdza składnię, nie rozwiązywalność typów z brakującego modułu,
a testy C++ nie ładują QML-a. Dlatego krok z prawdziwą aplikacją jest
obowiązkowy, a `liftplanner_qml_qmllint` łapie tę klasę błędu wcześniej.

**Diffów nie czytam.** Recenzja kodu wpuszczałaby do kontekstu setki linii co
obrót, a z `/usage` wynika, że to właśnie duży kontekst pali limit. Bramką
jakości są: testy jednostkowe developera, build bez ani jednego ostrzeżenia i
moja weryfikacja zachowaniowa w działającej aplikacji.

## Twarde zasady

- **Próg 85% okna 5-godzinnego** (podniesiony z 80% na prośbę użytkownika).
  Tygodniowy limit ignorujemy.
  `tmp/probe_usage.sh` kosztuje zero tokenów, więc sprawdzam co obrót.
- **Nigdy nie piszę do innych sesji** z `ListAgents` — to inne projekty
  użytkownika.
- **Sesję UI uruchamiam zawsze z `--port 49251`**, nie na domyślnym 49210. Na
  domyślnym porcie nasłuchuje `appfillin` z innego projektu użytkownika, a
  `ui_session.py start` adoptuje cudzą instancję zamiast wystartować naszą
  („already running, unknown pid"). `tmp/walk.py` łączy się na 49251.
  Cudzej instancji nigdy nie ubijam.
- **Nigdy nie ruszam prawdziwych danych**: `liftplanner.db` w korzeniu ani
  `~/.local/share/LiftPlanner/`. Sesja UI sama się piaskownicuje w `tmp/run`.
- **Nie pushuję.** Commity zostają lokalnie.
- **Buduję zawsze `-j 4`**, nigdy `-j$(nproc)` — użytkownik pracuje równolegle
  w innych oknach.
- Agentów trzymam przy życiu i odzywam się do nich `SendMessage` zamiast
  spawnować nowych — świeży agent to kilkanaście tysięcy tokenów na samo wejście.
  **Kompakcja kontekstu zabija agentów.** Po niej `SendMessage` odpowiada „No
  agent named … is reachable" i trzeba spawnować od nowa. Wtedy jednak
  `subagent_type: product-manager` / `developer` już działa (definicje z
  `.claude/agents/` wczytały się przy starcie sesji), więc obejście przez
  `general-purpose` czytający własną definicję jest niepotrzebne.
- Agenty mają myślenie włączone — dziedziczą je z sesji. Zmierzone na
  transkryptach iteracji 1 (`tmp/thinkcheck.py`): developer 28 bloków / 8352
  tokeny, product manager 26 bloków / 9318 tokenów. Poziom podbijam słowami
  kluczowymi w prompcie tam, gdzie liczy się projekt, nie wykonanie.
- Definicje z `.claude/agents/` wczytują się przy starcie sesji, więc w sesji, w
  której powstały, `subagent_type` ich nie zna. Obejście bez restartu: spawnuję
  `general-purpose` i w prompcie każę mu przeczytać
  `.claude/agents/<rola>.md` i przyjąć to jako swoją instrukcję. Przy okazji
  treść definicji nie wchodzi wtedy do mojego kontekstu.
- Obchód aplikacji leci przez `tmp/walk.py` — jedno wywołanie na całą ścieżkę,
  zwraca skrót plus podejrzane linie z logu, zamiast surowych dumpów. Zrzuty
  oglądam wybiórczo, bo obrazek to ponad tysiąc tokenów.
- **`click:` na bębnie i na przyciskach popupów nie wraca** — sterownik czeka na
  wyciszenie animacji, której bęben nie kończy. Kliknięcie wykonuje się
  poprawnie, wisi samo wywołanie. Robię wtedy jeden `click:` na wywołanie,
  po timeoucie `TaskStop`, a stan czytam osobnym wywołaniem. `set:` na
  `currentIndex` nie zadziała — Qt Quick Controls przyjmują tę zmianę tylko
  z interakcji użytkownika.
- Buduję przez `bash tmp/buildwarn.sh` (log do `tmp/build.log`, na wyjściu
  liczba ostrzeżeń), nie gołym `cmake --build` — ten wypluwa setki linii.

## Stan

Aktualizowany po każdym commicie — to jest pamięć pętli.

### Jak wznowić

Tura zamknięta **przy 79%** okna pięciogodzinnego (próg 85%, obrót kosztuje
5-6 punktów, więc następny nie zmieściłby się w całości). Drzewo czyste, nic nie
zostało w locie, wszystkie 22 pozycje zamknięte i zacommitowane.

1. `bash tmp/probe_usage.sh` — jeśli poniżej progu, ruszamy.
2. Agenty z tamtej sesji już nie żyją (kompakcja je zabija). Spawnuję na nowo
   przez `subagent_type: product-manager` i `developer` — definicje z
   `.claude/agents/` wczytują się przy starcie sesji, więc działają wprost.
3. Product manager ma gotowe zlecenie: **G13, pierwszy plaster — przełączanie
   wykonania pojedynczej serii w zakończonym treningu**. Sam je przepisał
   z „do rozbicia" na warunek akceptacji i postawił na czele „Open"; ocenił je
   jako jedyną dziurę stojącą w poprzek dorobku tej tury, bo dziś jedynym
   sposobem poprawienia zapisu jest skasowanie całej sesji.
4. Ocena stanu produktu po 22 iteracjach jest w `doc/product-gaps.md`, sekcja
   **„Stan produktu po 21 iteracjach"** (pisana przed ostatnim obrotem).

### Bilans tury

Dwadzieścia dwie pozycje zamknięte, testy **682 → 903**, pięć zawrotów do
developera — **wszystkie przy zielonym `ctest`**: G5 (brakujący import QML,
aplikacja nie wstawała), G23 (ekran nie odświeżał listy, baza była poprawna),
G8 (ucięta etykieta), G27 i pośrednio G32 (odmiana liczebników). Żadnego z nich
nie złapałby build ani testy jednostkowe.

- **Iteracja:** 22 zamknięta (G32 — jedno miejsce odmienia liczebniki)
- **W locie:** nic
- **Ostatni commit pętli:** G32
- **Najlepsze pozycje tej pętli wyszły z weryfikacji, nie z backlogu.** G27
  i G33 zobaczyłem, sprawdzając co innego (odpowiednio G4 i G26), i oba okazały
  się pilniejsze od wszystkiego, co wtedy leżało na liście. Dlatego w raporcie
  dla PM opisuję też to, czego nie szukałem — sekcja „Znalezisko" zarabia na
  siebie.
- **Piaskownica `tmp/run` spuchła do 163 treningów**, w większości duplikatów po
  trzech przejściach eksport → import. Świeżą daje usunięcie katalogu `tmp/run`
  (sesja UI odtworzy go z kopii bazy), ale przepadną stany zbudowane przez
  pętlę: przerwy z G6, dorzucone ćwiczenia z G9, nazwy z G16/G30, oraz oba
  przypadki flagowe, na których sprawdza się G31 i G33.
- **Schowek w sesji offscreen** żyje wewnątrz procesu, więc `xclip` go nie
  dosięga. Wstawianie treści: `set:notificationPopup.text=…`,
  `set:notificationPopup.copyEnabled=true`, `invoke:notificationPopup.open`,
  `click:notificationCopyButton`. Poprawny JSON historii najprościej wrzucić
  przyciskiem `exportHistoryButton`.
- **Sprawdzając zasiew z historii, nie ufaj danym, które zostawił developer.**
  Przy G9 pierwszy przebieg czytał wpis, który sam odłożył w `tmp/run`, i nie
  dowodził niczego. Dopiero ćwiczenie z historii **z importu, bez ani jednej
  flagi**, pokazało, że zasiew przechodzi przez amnestię z G19. Piaskownica jest
  wspólna, więc przed pomiarem sprawdzam, skąd wzięły się dane, na których mierzę.
- **Piaskownica po turze:** 164 treningi, aktywny „Base Strength" (22 serie,
  jedna odhaczona) — zostawiony celowo, bo to gotowy przypadek do G13.
- **Pułapka pustych flag `completed` — czwarte spotkanie** (G19, G23, G25, G31).
  Kształt bez ani jednego ptaszka nie znaczy „nic nie zrobione", tylko „flagi
  nic nie znaczą"; w piaskownicy leży `w4 Base Strength` z importu, 20 serii,
  zero flag. Reguła jest raz napisana w `src/domain/workout/performedsets.h`
  (`countsAsPerformed`, `completionFlagsAreMeaningful`) i drugiej kopii nie
  piszemy. Przy G31 pierwszy raz weszła do warunku akceptacji **z góry**,
  zamiast wyjść dopiero w testach — tak ma zostać: przy każdej pozycji
  dotykającej flag `completed` człon „a co z sesją bez ani jednej flagi"
  dopisujemy od razu.
- **Nieobecność elementu w drzewie nie jest dowodem usterki** — serwer
  automatyzacji przycina niewidoczne poddrzewa. Przy G6 odhaczyłem serię w
  treningu mającym **jedną** serię, timera nie było i o mało nie zawróciłem
  poprawnej roboty jako „przerwa nie działa". Nie ma po czym odpoczywać, więc
  paska nie ma. Zanim uznam brak za defekt, sprawdzam, czy scenariusz w ogóle
  wywołuje ten stan.
- **Gdy dwie pozycje ciągną ten sam układ w przeciwne strony**, warunek
  akceptacji tej wcześniejszej wchodzi do zlecenia następnej i sprawdzam oba
  naraz. Zrobione przy G30 wobec G28 (nazwa nie mogła urosnąć kosztem przycisku)
  — pomiary wyszły identyczne, więc regresji nie było, ale bez wpisania tego do
  zlecenia nikt by jej nie szukał.
- **Zrzut oglądam najpierw jako całość.** G28 (przycisk startu przecięty przez
  dolną nawigację) był na moich zrzutach z iteracji 12 i 13 i go nie zobaczyłem,
  bo za każdym razem szukałem na nich konkretnej rzeczy. Znalazł to dopiero
  własny obchód product managera.
- **Zawroty do developera:** 4, wszystkie przy zielonym `ctest`:
  G5 (brakujący import QML — aplikacja nie wstawała), G23 (lista zaplanowanych
  nieodświeżana po porzuceniu sesji — baza poprawna, ekran kłamał), G8 (etykieta
  „DURATION" ucięta do „DURATI…" — poprawne funkcjonalnie, brzydkie), G27
  („1 of 1 **sets** ticked off" — testy pilnowały liczb w tekście, nie tego, czy
  brzmi po ludzku).
  **Wniosek: żadnego z tych czterech nie złapałby build ani testy.** Dwa
  wymagały klikania w działającą aplikację, dwa przeczytania tego, co widzi
  użytkownik.
- **Pętla zatrzymana na progu** przy 82% okna 5-godzinnego (próg 85%, iteracja
  kosztuje 5-6 punktów, więc kolejna nie zmieściłaby się w całości). Drzewo
  czyste, nic nie zostało w locie. Wznowienie: `bash tmp/probe_usage.sh`, potem
  `SendMessage` do żywego product managera z prośbą o wybór pozycji — jeśli
  agenty już nie żyją, spawnować od nowa wg sekcji „Obsada".
- **Korekta roli PM (po uwadze użytkownika):** product manager zaczął zachowywać
  się jak analityk kodu — G19 i G21 to defekty spójności wyłowione z
  `workoutservice.cpp`, a nie braki, które czuje ktoś na siłowni. Definicja
  agenta przestawiona: punkt wyjścia to raport z aplikacji i przejście ekranów
  oczami użytkownika, kod wyłącznie do sprawdzenia, czy coś już istnieje.
  Niespójność trafia na backlog tylko wtedy, gdy człowiek widzi złą liczbę.
- **Zawroty do developera:** 1 (G5, brakujący import w QML)
- **Stan piaskownicy `tmp/run` zmieniony w iteracji 5:** „Regression Template"
  zakończony, doszedł trening „Seed check". Zrobione celowo, żeby powstała
  historia z ukończoną serią — bez tego nie dało się przetestować G15.

### Koszt: jak mierzyć, żeby wyszło prawdziwie

**Procentów z `/usage` NIE wolno używać do porównywania kosztu iteracji.** Limit
5-godzinny jest wspólny dla całego konta, więc te procenty zawierają spalanie
wszystkich równoległych okien użytkownika. Wcześniejsze porównanie „agenty
kosztują 3× tyle co robota własna" powstało właśnie tak i **jest nieuprawnione —
wycofane**. `/usage` zostaje wyłącznie do progu 80%, i tam jest właściwe, bo
limit faktycznie jest wspólny.

Do porównywania kosztu służą dwa źródła odporne na inne okna:

- `python3 tmp/usage.py` — mój własny transkrypt sesji, brany jako różnica
  „billed-ish" przed i po iteracji,
- `subagent_tokens` z powiadomienia o powrocie agenta — licznik **kumulatywny**
  za całe życie agenta, więc koszt jednej tury to różnica względem poprzedniego
  powrotu.

Zmierzone w iteracjach 1-5:

| | pierwsze uruchomienie | kolejne tury |
|---|---|---|
| product manager | 123k | +20k, +12k, +10k |
| developer | 111k | +46k, +42k, +21k, +63k |

**Wznowiony agent kosztuje ułamek świeżego spawnu.** Dlatego zawsze `SendMessage`
do żywego agenta, nigdy nowy spawn na kolejną iterację.

Cała pętla (iteracje 1-5): mój koszt 686k tokenów, kontekst urósł 74k → 287k;
agenty łącznie 447k we własnych kontekstach.
- **Odłożone:** nic
- **Agenty przy życiu:** product-manager i developer z iteracji 1 — odzywać się
  do nich `SendMessage`, nie spawnować nowych.
- **Raport testowy dla PM:** `tmp/loop/test-report.md` (nadpisywany co iterację)
