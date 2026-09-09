# Refactor TODO

Plan wykonania playbooka z `repaced` (`tmp/refactor-playbook.md` w tamtym repo)
w LiftPlannerze. Po każdym etapie: build, test, commit.

Weryfikacja: `bash tmp/verify.sh [configure|format|format_check|headers|build|test]`
(`tmp/` jest gitignorowane, skrypt trzeba odtworzyć w nowej sesji).

## Faza 0 — przygotowanie

- [x] Podbić `libs` do najświeższego mastera (projekcje SQL, read port, transakcje,
      `sqlcodec`, `upsert`). Ogarnąć `Task<T>` oznaczone `[[nodiscard]]`.
- [x] Jedno wejście do weryfikacji: `tmp/verify.sh`.

## Faza 1 — układ katalogów

- [x] Rozbić `src/modules/` na warstwy: `src/domain`, `src/infrastructure`,
      `src/application`, `src/ui`. Podział po temacie **wewnątrz** warstwy.
- [x] Zlikwidować `src/core/` — rozlać do warstw.
- [x] Composition root obok `main.cpp` (`liftplannerapplication.*` już tam był).
- [x] Drzewo testów odbite na warstwach.
- [x] Opisać layout w CLAUDE.md.

## Faza 2 — repozytoria: read port i projekcje

Wynik i decyzje: [read-port-adoption.md](read-port-adoption.md).

- [x] Plan adopcji read portu jako dokument (`doc/read-port-adoption.md`).
- [x] Wydzielić read port obok agregatowego (płaskie wiersze, nie agregaty).
- [x] Czytać wiersze zamiast agregatów tam, gdzie odbiorca tylko wyświetla
      (szablony i statystyki; listy historii świadomie zostają na agregatach).
- [x] `count`/`exists` — już są `SELECT COUNT` w toolkicie, nic do zrobienia.
- [x] Warunki zapytań builderami, nie sklejanym stringiem.
- [x] Jeden `SELECT` z joinami zamiast N+1 (podsumowania szablonów).
- [x] Zdjąć operacje listowe z repozytorium agregatowego szablonów.
- [x] Pojedyncze pola jednym `UPDATE` (`saveSet`) + stabilne id dzieci.
- [x] Kontrakt testowy portu na prawdziwej bazie.
- [x] Model uczy się id po zapisie (`WorkoutModel::setId`, partia 6 Fazy 4).

## Faza 3 — porządki wspólne

Wykonane w partiach Fazy 4 — szczegóły w
[consistency-review.md](consistency-review.md).

- [x] Wspólne helpery wierszy repozytoriów w jedno miejsce (`DbRows::toEntities`,
      prywatne `findBy(Where)`).
- [x] Nazwać powtarzane literały (`RestSeconds::inherited`, `Ordered`,
      `FilterField`, `BackendWorker::drain` zamiast magicznego 64).
- [x] Usunąć martwy kod i nieużywane konwersje (puste `validate()`; konwertery
      enumów mają wywołania — A9).
- [ ] Konwersje do/z SQL do toolkitu (`infrastructure/whereclause.h` — A14).
- [ ] Tabele poboczne w jedną klasę.
- [ ] Współdzielić listę tabel między testami schematu.

## Faza 4 — review spójności i naprawa partiami

- [x] Przeczytać całe `src` partiami, wnioski do `doc/consistency-review.md`,
      każdy oznaczony `[A]` / `[B]` / `[C]` / `[!]`. Nie poprawiać w trakcie.
- [x] Poprawić `[A]`, `[B]`, `[!]` partiami: domena → repozytoria → serwisy →
      view modele → composition root. Po każdej partii testy i commit.
      Dziewięć partii, A7 i A18 świadomie zostawione z uzasadnieniem w sekcji
      „Rules adopted while repairing".
- [x] Partia 10 — A20: encja w modelu to rekord skalarów, dzieci tylko w modelach
      dzieci. Zostaje większy rozdział ról modelu (projekcja do list i edytora vs
      mutowalny model aktywnego treningu) i mutowalne `exercises()`/`sets()`.

## Faza 5 — weryfikacja

- [x] Regresja pod automatyzacją UI (`tmp/ui_session`), log aplikacji czysty.
      Przeszło: start aplikacji, migracje 7 i 8 na kopii prawdziwej bazy, ekran
      główny ze statystykami z płaskich wierszy, aktywny trening (ukończenie
      serii — zapis wyłącznie `Upserted` i `Deleted 0 rows`), edytor (dodanie
      ćwiczenia, dodanie/duplikacja/usunięcie serii, stepper, zapis), picker
      z filtrem (90 → 5, trim), zapis szablonu i lista szablonów z podsumowaniem
      z jednego SELECT-a, wyszukiwanie szablonów, zamknięcie bez asercji.
      Przy okazji: `qrc:/LiftPlannerMain.qml` — brakujący ukośnik w korzeniu QML
      nie pozwalał aplikacji wystartować.

## Po refaktorze

- [x] Testy integracyjne konsumują `Task` — build bez ani jednego ostrzeżenia,
      aplikacja i testy razem.
- [x] Baza pod `AppDataLocation` zamiast katalogu roboczego procesu
      (`AppStoragePaths`), z jednorazowym przejęciem bazy leżącej obok
      aplikacji. Kopia, nie przeniesienie — oryginał zostaje.
- [ ] Rozdział ról `WorkoutModel`: projekcja tylko-do-odczytu dla list i edytora
      vs mutowalny model aktywnego treningu. Zamyka mutowalne
      `Workout::exercises()` / `Exercise::sets()` i podwójny stan w edytorze.
- [ ] Do decyzji: edytor serii w aktywnym treningu otwiera się wyłącznie długim
      przytrzymaniem i nic tego nie sygnalizuje.

## Pułapki

- Klasy rejestrowane w QML nie mogą być `final`.
- Nowy plik `.cpp`/`.qml` → ponowne `cmake -S . -B build-desktop`.
- Listy `SOURCES` w `CMakeLists.txt` testów są ręczne — glob ich nie dotyczy.
- `Task<T>` jest `[[nodiscard]]`; kontekst `then`/`onError` musi przeżyć zadanie,
  więc w ścieżkach z destruktora używać `warnOnError(const char*)` (kontekst =
  worker), nie `warnOnError(this, ...)`.
