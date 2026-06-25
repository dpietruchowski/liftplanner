# lift-planner — dokumentacja

Qt6/QML workout planner (C++20, SQLite via QtSql, CMake + Ninja). Buduje się na desktop
(Linux) i Android (APK).

## Indeks

- [architecture.md](architecture.md) — warstwy, przepływ danych, composition root, rola `libs`.
- [modules.md](modules.md) — moduły `workout` i `userprofile`: encje, serwisy, repozytoria.
- [data-model.md](data-model.md) — schemat SQLite (tabele, relacje, migracje).
- [testing.md](testing.md) — budowanie i uruchamianie testów, `TestApplication`, time-travel, automatyzacja UI.
- [test-cases/](test-cases/) — scenariusze smoke pod driver automatyzacji UI.

## Szybki start

```bash
cmake -S . -B build-desktop -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-desktop
ctest --test-dir build-desktop
./build-desktop/src/liftplanner
```

## Powiązane

- [.claude/TODO.md](../.claude/TODO.md) — backlog funkcjonalny i blockery przed wydaniem.
- Skille `.claude/skills/` (`format-code`, `commit`, `todo`, `ui-session`) i agenci
  `.claude/agents/` (`scout`, `committer`).

> Pliki [architecture-for-llm.md](architecture-for-llm.md) i
> [project-context.md](project-context.md) to starsze, generyczne materiały szablonowe —
> aktualny, wierny kodowi opis jest w plikach powyżej.
