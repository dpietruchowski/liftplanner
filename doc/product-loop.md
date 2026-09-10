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
4. **Ja — test.** Buduję, `ctest`, potem sesja UI: przechodzę ścieżkę
   użytkownika, sprawdzam warunek akceptacji, robię zrzut, czytam log aplikacji.
   Wynik zapisuję do `tmp/loop/test-report.md`.
   Nie działa → wracam do developera z konkretem, maksymalnie dwa razy; za
   trzecim odkładam pozycję i notuję dlaczego.
5. **Ja — commit.** `git commit -- <ścieżki>`, tryb rozkazujący, bez trailerów.
   Ścieżki biorę z raportu developera i z `git status --porcelain`.
   Odhaczam pozycję w `product-gaps.md` i aktualizuję stan na dole.
6. Wracam do 1.

**Diffów nie czytam.** Recenzja kodu wpuszczałaby do kontekstu setki linii co
obrót, a z `/usage` wynika, że to właśnie duży kontekst pali limit. Bramką
jakości są: testy jednostkowe developera, build bez ani jednego ostrzeżenia i
moja weryfikacja zachowaniowa w działającej aplikacji.

## Twarde zasady

- **Próg 80% okna 5-godzinnego.** Tygodniowy limit ignorujemy.
  `tmp/probe_usage.sh` kosztuje zero tokenów, więc sprawdzam co obrót.
- **Nigdy nie piszę do innych sesji** z `ListAgents` — to inne projekty
  użytkownika.
- **Nigdy nie ruszam prawdziwych danych**: `liftplanner.db` w korzeniu ani
  `~/.local/share/LiftPlanner/`. Sesja UI sama się piaskownicuje w `tmp/run`.
- **Nie pushuję.** Commity zostają lokalnie.
- **Buduję zawsze `-j 4`**, nigdy `-j$(nproc)` — użytkownik pracuje równolegle
  w innych oknach.
- Agentów trzymam przy życiu i odzywam się do nich `SendMessage` zamiast
  spawnować nowych — świeży agent to kilkanaście tysięcy tokenów na samo wejście.
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

- **Iteracja:** 1 zamknięta (G1 — edycja zaplanowanego treningu)
- **W locie:** nic
- **Ostatni commit pętli:** G1
- **Odłożone:** nic
- **Agenty przy życiu:** product-manager i developer z iteracji 1 — odzywać się
  do nich `SendMessage`, nie spawnować nowych.
- **Raport testowy dla PM:** `tmp/loop/test-report.md` (nadpisywany co iterację)
