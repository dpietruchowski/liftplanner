# Smoke 01 — start i ukończenie treningu

Scenariusz przez UI driver. Sprawdza pełny cykl aktywnego treningu: wybór
zaplanowanego treningu → start → oznaczanie serii jako wykonanych → zakończenie →
trening trafia do historii.

## Cel

Po zakończeniu trening ze statusu `Active` przechodzi w `Ended`, zostaje zapisany do
bazy i jest widoczny w historii na ekranie Home. Aktywność tygodniowa zaznacza dzień
treningu.

## Wymagania wstępne

- Aplikacja postawiona (`ui_session.py start`).
- Na liście Planned istnieje co najmniej jeden zaplanowany trening. Jeśli nie — wykonaj
  najpierw [smoke-02](smoke-02-plan-and-import-workout.md).
- Ustalony zegar dla powtarzalności: `set_time 2026-01-05` (poniedziałek).

## Mapa kontrolek (potwierdź `find`/`dump`)

| Ekran | objectName | Do czego |
|-------|------------|----------|
| Nawigacja | `navWorkoutsButton` (?) / `navHomeButton` (?) | przełączanie ekranów |
| Workouts | `screenWorkouts` | lista zaplanowanych |
| Workouts | `startWorkoutButton` (?) | start wybranego treningu |
| Active | `screenActiveWorkout` | ekran aktywnego treningu |
| Active | `completeSetButton` (?) | oznacz bieżącą serię jako wykonaną |
| Active | `nextButton` (?) / `previousButton` (?) | nawigacja po seriach/ćwiczeniach |
| Active | `endWorkoutButton` (?) | zakończ trening |
| Home | `screenHome` | historia + aktywność tygodniowa |

## Procedura

1. **Ustaw zegar i wejdź na Workouts.**
   ```
   set_time 2026-01-05
   click navWorkoutsButton
   dump
   ```
   Oczekiwane: widoczny `screenWorkouts` z co najmniej jednym zaplanowanym treningiem.

2. **Wystartuj trening.**
   ```
   click startWorkoutButton
   dump
   ```
   Oczekiwane: widoczny `screenActiveWorkout`; `ActiveWorkoutViewModel` ma `isActive=true`
   i ustawiony `currentWorkout`. (Potwierdź `get screenActiveWorkout visible`/`dump`.)

3. **Ukończ kolejne serie.** Dla każdej serii:
   ```
   click completeSetButton
   click nextButton
   ```
   Powtarzaj aż wszystkie serie wszystkich ćwiczeń będą oznaczone (`dump` pokazuje serie
   jako `[checked]`).

4. **Zakończ trening.**
   ```
   click endWorkoutButton
   dump
   ```
   Oczekiwane: powrót z ekranu aktywnego; `isActive=false`.

5. **Sprawdź historię.**
   ```
   click navHomeButton
   dump
   ```
   Oczekiwane: trening pojawia się jako ostatni w historii (`lastWorkout`), a kafelek
   aktywności tygodniowej ma zaznaczony poniedziałek (indeks 0 z 7).

6. **Sprzątanie.**
   ```
   reset_time
   ```

## Oczekiwany stan końcowy

- Trening ma status `Ended` i `ended_time` ustawiony (zapisany w `workouts`).
- Pojawia się w historii na `screenHome`; `weekActivity` zaznacza dzień treningu.
- Lista Planned nie pokazuje już tego treningu jako oczekującego.

## Uwagi

- Dokładne `objectName` oznaczone `(?)` potwierdź przez `find`/`dump` — to skeleton, a
  driver jest źródłem prawdy.
- Jeśli ukończenie treningu ma odświeżyć historię reaktywnie, krok 5 nie powinien
  wymagać ręcznego przeładowania — `WorkoutHistoryViewModel` nasłuchuje
  `ActiveWorkoutViewModel::workoutCompleted`.
