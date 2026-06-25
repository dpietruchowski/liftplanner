# Test-cases — scenariusze smoke (UI driver)

Scenariusze prowadzone **wyłącznie przez UI** za pomocą drivera automatyzacji
([tools/ui_automation/](../../tools/ui_automation/)). Każdy krok to komenda
`liftplanner_driver.py`, a po niej oczekiwany stan. Zasady ogólne i pełna lista komend:
skill `ui-session` oraz [doc/testing.md](../testing.md).

## Jak uruchamiać

```bash
# 1. Zbuduj desktop, jeśli trzeba
cmake -S . -B build-desktop -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build-desktop

# 2. Postaw instancję (idempotentne — adoptuje działającą na porcie 49210)
python3 tools/ui_automation/liftplanner_session.py start

# 3. Wykonuj kroki scenariusza, każdy to jedno wywołanie:
python3 tools/ui_automation/liftplanner_driver.py <komenda> <args…>
```

## Odkrywanie kontrolek

**Nie zgaduj `objectName` z kodu** — odkrywaj je z działającej aplikacji:

```bash
python3 tools/ui_automation/liftplanner_driver.py find ""          # wszystkie nazwy
python3 tools/ui_automation/liftplanner_driver.py find "workout"    # filtr po podłańcuchu
python3 tools/ui_automation/liftplanner_driver.py dump              # drzewo widocznych + właściwości
```

Główne ekrany wystawiają nazwy w stylu `screenHome`, `screenWorkouts`,
`screenActiveWorkout`, `screenProfile` oraz przyciski dolnej nawigacji (`nav*Button`).
W scenariuszach `objectName` oznaczone `(?)` to **kandydaci do potwierdzenia** przez
`find`/`dump` w danej wersji UI — driver jest źródłem prawdy, nie ten dokument.

## Determinizm czasu

Aktywność tygodniowa i podział planned/history zależą od zegara. Przed scenariuszem
ustaw datę (`set_time <iso>`), a do przesuwania użyj `advance_time <dni>`; na końcu
`reset_time`. Zegar runtime idzie przez `TimeProvider`, więc UI reaguje na mock tak
samo jak testy integracyjne.

## Scenariusze

- [smoke-01-start-and-finish-workout.md](smoke-01-start-and-finish-workout.md) — start
  zaplanowanego treningu, ukończenie serii, zakończenie, zapis do historii.
- [smoke-02-plan-and-import-workout.md](smoke-02-plan-and-import-workout.md) — wygenerowanie
  promptu z profilu, import planu z JSON (schowek), pojawienie się na liście Planned.
- [smoke-03-history-and-weekly-activity.md](smoke-03-history-and-weekly-activity.md) —
  historia i kafelek aktywności tygodniowej, reakcja na zmianę zegara.
