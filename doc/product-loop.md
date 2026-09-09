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
- Agentów trzymam przy życiu i odzywam się do nich `SendMessage` zamiast
  spawnować nowych — świeży agent to kilkanaście tysięcy tokenów na samo wejście.
- Obchód aplikacji leci przez skrypt zwracający skrót, nie przez surowe dumpy.
  Zrzuty oglądam wybiórczo, bo obrazek to ponad tysiąc tokenów.

## Stan

Aktualizowany po każdym commicie — to jest pamięć pętli.

- **Iteracja:** 0 (jeszcze nie ruszyła)
- **W locie:** nic
- **Ostatni commit pętli:** brak
- **Odłożone:** nic
