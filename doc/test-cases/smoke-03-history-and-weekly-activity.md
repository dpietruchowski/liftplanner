# Smoke 03 — historia i aktywność tygodniowa

Scenariusz przez UI driver. Sprawdza ekran Home: listę historii, kafelek aktywności
tygodniowej oraz jego reakcję na zmianę zegara (`TimeProvider`).

## Cel

Aktywność tygodniowa (`weekActivity`, 7 booleanów Pon–Nd) zaznacza dni z treningami z
bieżącego tygodnia, łącznie z aktualnie aktywnym treningiem. Po przesunięciu zegara na
kolejny tydzień zaznaczenia odpowiednio się zmieniają.

## Kontekst

`weekActivity` liczy się od poniedziałku bieżącego tygodnia wg
`TimeProvider::instance().currentDate()` i uwzględnia zarówno zapisane treningi, jak i
`currentWorkout` z `ActiveWorkoutViewModel`
([workouthistoryviewmodel.cpp:115-139](../../src/ui/viewmodels/workouthistoryviewmodel.cpp#L115-L139)).
Kafelek odświeża się reaktywnie przez sygnały (`workoutsChanged`,
`ActiveWorkoutViewModel::currentWorkoutChanged`).

## Wymagania wstępne

- Aplikacja postawiona (`ui_session.py start`).
- W historii jest co najmniej jeden trening w bieżącym (zmockowanym) tygodniu — np. po
  wykonaniu [smoke-01](smoke-01-start-and-finish-workout.md) z `set_time 2026-01-05`.

## Mapa kontrolek (potwierdź `find`/`dump`)

| Ekran | objectName | Do czego |
|-------|------------|----------|
| Nawigacja | `navHomeButton` (?) | wejście na Home |
| Home | `screenHome` | historia + aktywność tygodniowa |
| Home | `weekActivityTile` (?) | kafelek 7 dni |
| Home | `lastWorkoutCard` (?) | ostatni trening |

## Procedura

1. **Ustaw zegar na środek tygodnia i wejdź na Home.**
   ```
   set_time 2026-01-07
   click navHomeButton
   dump
   ```
   Oczekiwane: historia pokazuje trening z 2026-01-05 (poniedziałek); kafelek aktywności
   ma zaznaczony indeks 0 (poniedziałek), resztę odznaczoną.

2. **Sprawdź odczyt aktywności.**
   ```
   get weekActivityTile <wlasciwosc>
   ```
   lub odczytaj z `dump`. Potwierdź wektor 7 wartości z poprawnie zaznaczonym dniem.

3. **Przesuń zegar na kolejny tydzień.**
   ```
   advance_time 7
   dump
   ```
   Oczekiwane: nowy tydzień nie ma jeszcze treningów — wszystkie 7 pozycji odznaczone;
   trening z poprzedniego tygodnia nadal jest w historii (lista historii nie znika,
   zmienia się tylko bieżące okno tygodnia).

4. **(Opcjonalnie) zaznacz dzień bieżącego tygodnia.** Wykonaj trening wg
   [smoke-01](smoke-01-start-and-finish-workout.md) w nowym tygodniu i sprawdź, że
   odpowiedni dzień zapala się w kaflu **bez** ręcznego przeładowania (reaktywność przez
   sygnały).

5. **Sprzątanie.**
   ```
   reset_time
   ```

## Oczekiwany stan końcowy

- Lista historii zawiera wszystkie zapisane treningi niezależnie od bieżącego tygodnia.
- `weekActivity` odzwierciedla tylko bieżący (zmockowany) tydzień i aktualizuje się po
  `set_time`/`advance_time`.

## Uwagi

- `objectName` z `(?)` potwierdź przez `find`/`dump`.
- To dobry scenariusz na regresję determinizmu czasu — jeśli kafelek nie reaguje na
  `advance_time`, prawdopodobnie gdzieś użyto `QDateTime::currentDateTime()` zamiast
  `TimeProvider`.
