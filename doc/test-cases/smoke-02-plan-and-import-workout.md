# Smoke 02 — zaplanowanie i import treningu (JSON z AI)

Scenariusz przez UI driver. Sprawdza ścieżkę planowania: profil → prompt do schowka →
wklejenie odpowiedzi modelu (JSON) → import → trening na liście Planned.

## Cel

Po imporcie poprawnego JSON-a plan treningowy pojawia się jako zaplanowany trening
(status `Planned`) z ćwiczeniami i seriami, gotowy do wystartowania
([smoke-01](smoke-01-start-and-finish-workout.md)).

## Kontekst

Aplikacja nie woła LLM bezpośrednio. Prompt budowany jest z profilu i szablonu
[src/data/gpt_prompt_template.txt](../../src/data/gpt_prompt_template.txt), kopiowany do
schowka; użytkownik wkleja odpowiedź modelu jako JSON, który parsuje
[workoutjson](../../src/utils/workoutjson.h) i importuje
`WorkoutService::importPlannedWorkouts`. W teście pomijamy realny model — podajemy gotowy
JSON.

## Wymagania wstępne

- Aplikacja postawiona (`ui_session.py start`).
- Profil użytkownika istnieje (jeśli nie — ustaw pola na `screenProfile`).
- Przykładowy JSON planu zgodny z formatem `workoutjson` (tablica treningów).

## Mapa kontrolek (potwierdź `find`/`dump`)

| Ekran | objectName | Do czego |
|-------|------------|----------|
| Nawigacja | `navWorkoutsButton` (?) / `navProfileButton` (?) | przełączanie ekranów |
| Profil | `screenProfile` | edycja profilu zasilającego prompt |
| Workouts | `screenWorkouts` | lista zaplanowanych |
| Workouts | `generatePromptButton` (?) | zbuduj prompt i skopiuj do schowka |
| Workouts | `importFromClipboardButton` (?) | import JSON ze schowka |

## Procedura

1. **(Opcjonalnie) ustaw profil.**
   ```
   click navProfileButton
   dump
   ```
   Ustaw cel/poziom/sesje przez `set <name> <property> <value>` wg `dump`, jeśli puste.

2. **Wygeneruj prompt.**
   ```
   click navWorkoutsButton
   click generatePromptButton
   ```
   Oczekiwane: prompt trafia do schowka (potwierdzenie w UI). Treść promptu zawiera dane
   profilu.

3. **Wklej odpowiedź modelu do schowka (z zewnątrz).** W realnym przebiegu kopiujesz
   odpowiedź z modelu. W teście wstaw przygotowany JSON do schowka systemowego, np.:
   ```bash
   printf '%s' "$PLAN_JSON" | xclip -selection clipboard
   ```
   gdzie `PLAN_JSON` to poprawna tablica treningów.

4. **Zaimportuj.**
   ```
   click importFromClipboardButton
   dump
   ```
   Oczekiwane: nowy trening pojawia się na liście Planned z ćwiczeniami i seriami.
   Przy niepoprawnym JSON-ie viewmodel emituje `errorOccurred` — UI pokazuje komunikat,
   a lista się nie zmienia (to dobry test negatywny).

## Oczekiwany stan końcowy

- Zaimportowany trening ma status `Planned` i jest widoczny na `screenWorkouts`.
- Liczba ćwiczeń/serii odpowiada wejściowemu JSON-owi.
- Można go wystartować — przejście do [smoke-01](smoke-01-start-and-finish-workout.md).

## Uwagi

- `objectName` z `(?)` potwierdź przez `find`/`dump`.
- Import jest też dostępny z surowego JSON-a (`importFromJson`) — można go wywołać
  bezpośrednio przez `invoke`, pomijając schowek, jeśli wariant ze schowkiem jest
  niewygodny w danym środowisku.
