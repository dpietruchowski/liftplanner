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

- [ ] Plan adopcji read portu jako dokument (`doc/read-port-adoption.md`).
- [ ] Wydzielić read port obok agregatowego (płaskie wiersze, nie agregaty).
- [ ] Czytać wiersze zamiast agregatów tam, gdzie odbiorca tylko wyświetla.
- [ ] `count`/`exists`/`findIds` na projekcji jednej kolumny.
- [ ] Warunki zapytań builderami, nie sklejanym stringiem.
- [ ] Jeden `SELECT` z joinami zamiast N+1.
- [ ] Zdjąć operacje listowe z repozytorium agregatowego.
- [ ] Pojedyncze pola jednym `UPDATE`.
- [ ] Testy portu na prawdziwej bazie przez fixture; kontrakt testowy portu.

## Faza 3 — porządki wspólne

- [ ] Wspólne helpery wierszy repozytoriów w jedno miejsce.
- [ ] Konwersje do/z SQL do toolkitu.
- [ ] Tabele poboczne w jedną klasę.
- [ ] Nazwać powtarzane literały.
- [ ] Helpery walidacyjne zamiast łańcuchów `if (...) throw`.
- [ ] Usunąć martwy kod i nieużywane konwersje.
- [ ] Współdzielić listę tabel między testami schematu.

## Faza 4 — review spójności i naprawa partiami

- [ ] Przeczytać całe `src` partiami, wnioski do `doc/consistency-review.md`,
      każdy oznaczony `[A]` / `[B]` / `[C]` / `[!]`. Nie poprawiać w trakcie.
- [ ] Poprawić `[A]`, `[B]`, `[!]` partiami: domena → repozytoria → serwisy →
      view modele → composition root. Po każdej partii testy i commit.

## Faza 5 — weryfikacja

- [ ] Regresja pod automatyzacją UI (`tmp/ui_session`), czytać log aplikacji.

## Pułapki

- Klasy rejestrowane w QML nie mogą być `final`.
- Nowy plik `.cpp`/`.qml` → ponowne `cmake -S . -B build-desktop`.
- Listy `SOURCES` w `CMakeLists.txt` testów są ręczne — glob ich nie dotyczy.
- `Task<T>` jest `[[nodiscard]]`; kontekst `then`/`onError` musi przeżyć zadanie,
  więc w ścieżkach z destruktora używać `warnOnError(const char*)` (kontekst =
  worker), nie `warnOnError(this, ...)`.
