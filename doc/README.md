# lift-planner — dokumentacja

Qt6/QML workout planner (C++20, SQLite via QtSql, CMake + Ninja). Buduje się na desktop
(Linux) i Android (APK).

## Indeks

- [architecture.md](architecture.md) — warstwy, przepływ danych, composition root, rola `libs`.
- [modules.md](modules.md) — moduły `workout` i `userprofile`: encje, serwisy, repozytoria.
- [data-model.md](data-model.md) — schemat SQLite (tabele, relacje, migracje).
- [testing.md](testing.md) — budowanie i uruchamianie testów, `TestApplication`, time-travel, automatyzacja UI.
- [test-cases/](test-cases/) — scenariusze smoke pod driver automatyzacji UI.
- [garmin-integration.md](garmin-integration.md) — opcje integracji z Garmin Forerunner 970 bez aplikacji na zegarku.

## Szybki start

```bash
cmake -S . -B build-desktop -DCMAKE_BUILD_TYPE=Debug
cmake --build build-desktop
ctest --test-dir build-desktop
./build-desktop/src/appliftplanner
```

Parametry aplikacji (nazwa, target, wersja, pakiet Android, metadane AppImage) są w
[app.env](../app.env) — czyta je i CMake (`app_load_env()`), i skrypty z
`libs/scripts/`.

## Powiązane

- [.claude/TODO.md](../.claude/TODO.md) — backlog funkcjonalny i blockery przed wydaniem.
- Skille `.claude/skills/` (`format-code`, `commit`, `todo`, `ui-session`,
  `bump-android-version`) i agenci `.claude/agents/` (`scout`, `committer`) — wszystkie
  poza `scout` to dowiązania do `libs/claude/`.

> Pliki [architecture-for-llm.md](architecture-for-llm.md) i
> [project-context.md](project-context.md) to starsze, generyczne materiały szablonowe —
> aktualny, wierny kodowi opis jest w plikach powyżej.
