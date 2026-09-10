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

1. **Limit.** `bash tmp/probe_usage.sh`. Kod wyjścia 1 albo słowo `STOP` →
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

- **Próg 80% okna 5-godzinnego.** Tygodniowy limit ignorujemy.
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
- Buduję przez `bash tmp/buildwarn.sh` (log do `tmp/build.log`, na wyjściu
  liczba ostrzeżeń), nie gołym `cmake --build` — ten wypluwa setki linii.

## Stan

Aktualizowany po każdym commicie — to jest pamięć pętli.

- **Iteracja:** 4 zamknięta (G5 — edytor serii w aktywnym treningu)
- **W locie:** nic
- **Ostatni commit pętli:** G5
- **Zawroty do developera:** 1 (G5, brakujący import w QML)

### Koszt: agenty kontra robota własna

Zmierzone na sondzie `/usage`, w punktach okna 5-godzinnego:

| iteracja | pozycja | rozmiar | tryb | koszt |
|---|---|---|---|---|
| 1 | G1 | M | agenty | 7 pkt (PM ~3) |
| 2 | G7 | S | agenty | 6 pkt (PM 2, developer + testy 4) |
| 3 | G3 | S | solo | **2 pkt** |

Przy tym samym rozmiarze pozycji (S) agenty kosztują trzy razy tyle. Ale to nie
jest stała: robota własna wpycha treść plików do kontekstu głównej sesji na
stałe, więc jej koszt rośnie z każdą iteracją i kończy się kompakcją, podczas
gdy agenty trzymają ten kontekst płaski. Dwie różne krzywe, nie dwie liczby —
zbierać dalej i porównać po kilku iteracjach, a nie po jednej.
- **Odłożone:** nic
- **Agenty przy życiu:** product-manager i developer z iteracji 1 — odzywać się
  do nich `SendMessage`, nie spawnować nowych.
- **Raport testowy dla PM:** `tmp/loop/test-report.md` (nadpisywany co iterację)
