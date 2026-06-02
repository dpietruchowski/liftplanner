# 📋 TODO — funkcjonalne

> Stan zweryfikowany bezpośrednio w kodzie (viewmodele, serwisy, QML, manifest, gradle, CMake), nie przepisany ze starego `doc/todo.md`.

## 🔴 High priority
- [ ] **Brak ręcznego tworzenia/edycji treningów i ćwiczeń.** Cały przepływ planowania opiera się wyłącznie na imporcie JSON z AI ([plannedworkoutviewmodel.cpp](../src/ui/viewmodels/plannedworkoutviewmodel.cpp)). W aktywnym treningu da się tylko duplikować/usuwać serie ([activeworkoutviewmodel.h:32-33](../src/ui/viewmodels/activeworkoutviewmodel.h#L32-L33)) — **nie ma** `addExercise`/`addSet`/`createWorkout` w żadnym viewmodelu. Bez AI użytkownik nie zbuduje treningu ręcznie.
- [ ] **Brak UI do edycji profilu użytkownika.** `UserProfileService` istnieje i zasila prompt GPT, ale profil zapisywany jest wyłącznie „okrężnie” przy imporcie JSON z AI ([plannedworkoutviewmodel.cpp:71-77](../src/ui/viewmodels/plannedworkoutviewmodel.cpp#L71-L77)). Nie jest zarejestrowany jako singleton QML ([liftplannerapplication.cpp:41-44](../src/liftplannerapplication.cpp#L41-L44)) i nie ma ekranu profilu — użytkownik nie ustawi płci/celu/poziomu/daty urodzenia/wagi bezpośrednio.

## 🟡 Medium priority
- [ ] **Brak schematu wersjonowania bazy (`PRAGMA user_version`).** Jest tylko doraźna migracja kolumny `status` przez `table_info` + `ALTER` ([workoutrepositorydb.cpp:52-71](../src/modules/workout/infrastructure/database/workoutrepositorydb.cpp#L52-L71)); reszta to `createTable`. Dla pierwszej instalacji OK, ale przy kolejnych aktualizacjach brak ramy migracji jest ryzykowny — warto dodać `user_version` przed publikacją aktualizacji.
- [ ] **Brak podglądu wartości serii z poprzedniej sesji** podczas aktywnego treningu (potrzebne do progresji). Ekran pokazuje tylko bieżące serie ([ScreenActiveWorkout.qml](../src/ui/ScreenActiveWorkout.qml)).
- [ ] **Spłaszczone przyciski na Androidzie** — sizing do poprawy (np. `WorkoutButtonsRow`, przyciski +/-).
- [ ] **Układ dialogu „prompt skopiowany”** — komunikat/kolejność przycisków (copy nad OK) wygląda dziwnie ([MainView.qml:29-50](../src/ui/MainView.qml#L29-L50)).
- [ ] **Mieszany język UI** — etykiety nawigacji po angielsku („Workout/Home/Planned/History” w [MainView.qml:60-65](../src/ui/MainView.qml#L60-L65)), a projekt zakłada UI po polsku. Ustalić jeden język lub dodać i18n.

## 🔵 Low priority
- [ ] **Link YouTube ćwiczenia pokazany jako zwykły tekst**, nie klikalny ([ScreenActiveWorkout.qml:54-56](../src/ui/ScreenActiveWorkout.qml#L54-L56)). Można zrobić go otwieralnym (`Qt.openUrlExternally` — nie wymaga uprawnienia INTERNET).
- [ ] Animacje przy rozwijaniu/zwijaniu ćwiczeń.
- [ ] Widok kalendarza zaplanowanych treningów.

## ❓ Do ustalenia
- [ ] **Gdzie umieścić wejście do Profilu / Ustawień / About.** Dolna nawigacja ma już 4 sloty ([MainView.qml:60-65](../src/ui/MainView.qml#L60-L65)) — potrzebny 5. element, menu „hamburger” albo ekran ustawień. To samo miejsce może hostować ekran licencji (patrz blockery).
- [ ] **Czy w 1.0 ma być ręczna edycja treningów**, czy świadomie zostaje tryb „tylko import z AI”. Decyzja wpływa na zakres High-priority powyżej.

---

# 🚀 Blockery przed wydaniem na Google Play

## ⛔ Krytyczne (blokują publikację / aktualizacje)
- [ ] **Build produkuje APK, nie AAB.** Google Play wymaga Android App Bundle dla nowych aplikacji. CMake woła `qt_android_add_apk_target` ([CMakeLists.txt:46](../CMakeLists.txt#L46)) i `sign.sh` podpisuje `...-release-unsigned.apk` ([sign.sh](../sign.sh)). Trzeba zbudować i podpisać `.aab` (np. zadanie `bundleRelease` / `androiddeployqt --aab`).
- [ ] **`targetSdkVersion=34` — Play wymaga ≥35.** Finalny manifest pakuje `targetSdkVersion="34"` (sterowane przez `qtTargetSdkVersion=34`, [gradle.properties](../build-android/src/android-build/gradle.properties#L29), domyślne dla Qt 6.8). Ustawić `QT_ANDROID_TARGET_SDK_VERSION` na 35 (docelowo 36) w [CMakeLists.txt](../CMakeLists.txt).
- [ ] **Wersja: manifest nadpisuje CMake i `versionCode` nie rośnie.** Spakowany manifest ma `versionCode="1"`, `versionName="1.0"` — wartości z CMake (`QT_ANDROID_VERSION_CODE=3`, `1.0-rc2`, [CMakeLists.txt:13-14](../CMakeLists.txt#L13-L14)) są **ignorowane**, bo manifest ma zaszyte literały ([AndroidManifest.xml:5-6](../android/AndroidManifest.xml#L5-L6)). Każda aktualizacja w Play musi mieć rosnący `versionCode`; do tego „1.0-rc2” to nazwa release-candidate, nie produkcyjna. Ujednolicić jedno źródło prawdy (placeholdery `${...}` albo usunąć z manifestu i polegać na CMake).
- [ ] **Podpisywanie nieprodukcyjne.** `generate_key.sh` i `sign.sh` mają **zaszyte hasło** keystore `liftplanner123` oraz zaszyte ścieżki kontenera ([sign.sh](../sign.sh), [generate_key.sh](../generate_key.sh)). Keystore `android/lift-planner.keystore` jest w drzewie roboczym (ignorowany przez `.gitignore`, więc nie w repo — OK). Przed wydaniem: zmienić/wyprowadzić hasło poza skrypty, skonfigurować podpis AAB i włączyć **Play App Signing** (klucz upload). Zgubienie tego keystore = brak aktualizacji.

## 🟠 Wymagane porządki w manifeście
- [ ] **Usunąć `WRITE_EXTERNAL_STORAGE` + `requestLegacyExternalStorage`.** Aplikacja zapisuje tylko do `AppDataLocation` (pamięć wewnętrzna) ([activeworkoutviewmodel.cpp:37](../src/ui/viewmodels/activeworkoutviewmodel.cpp#L37), [:65](../src/ui/viewmodels/activeworkoutviewmodel.cpp#L65)). Te wpisy są ignorowane od API 30+ i źle wyglądają w Data Safety ([AndroidManifest.xml:9](../android/AndroidManifest.xml#L9), [:23](../android/AndroidManifest.xml#L23)).
- [ ] **Zweryfikować/usunąć `INTERNET`.** Brak realnych wywołań sieciowych (brak `QNetworkAccessManager`; link YouTube to czysty tekst; `UrlHelper` nie jest nawet zarejestrowany w QML) ([AndroidManifest.xml:8](../android/AndroidManifest.xml#L8)).
- [ ] **Zweryfikować/usunąć `ACCESS_NETWORK_STATE`** — w kodzie nie ma odpytywania stanu sieci ([AndroidManifest.xml:10](../android/AndroidManifest.xml#L10)).
- [ ] **`android:label="liftplanner"`** — mała litera; ustawić poprawną nazwę wyświetlaną „Lift Planner” ([AndroidManifest.xml:18](../android/AndroidManifest.xml#L18)).
- [ ] **`allowBackup="true"`** — zdecydować: baza SQLite z danymi treningowymi jest auto-backupowana do chmury Google. Rozważyć wyłączenie lub reguły backupu; wpływa na Data Safety ([AndroidManifest.xml:20](../android/AndroidManifest.xml#L20)).
- [ ] **Usunąć martwy kod z innej aplikacji.** `androidnotificationscheduler.cpp` odwołuje się do `com.fillin.app` i `NotificationReceiver`, który nie istnieje; kompiluje się do builda Androida, ale **nigdy nie jest wołany** ([androidnotificationscheduler.cpp:18](../libs/utils/android/androidnotificationscheduler.cpp#L18)). Usunąć — myli i sugerowałby `SCHEDULE_EXACT_ALARM` (którego słusznie nie ma).
- [ ] **Ikona aplikacji** — tylko stare `ic_launcher.png` w mipmapach, brak adaptive icon (foreground/background) i monochrome. Niezalecane, ale nie blokuje twardo ([android/res/](../android/res/)).

## ⚖️ Licencje / zgodność prawna
- [ ] **Brak atrybucji Qt (LGPLv3) i ekranu „About/Licenses”.** W całym repo nie ma tekstu licencji ani ekranu About. Qt na Androidzie linkowane jest dynamicznie (`.so`) — wymóg relinkowania OK — ale trzeba **dołączyć tekst licencji Qt + atrybucję** w aplikacji (ekran About) oraz notę o LGPL. Brak tego to luka prawna.
- [ ] **Potwierdzić licencje ikon SVG** ([src/ui/icons/](../src/ui/icons/), [libs/icons/](../libs/icons/)) — jeśli pochodzą z pakietu ikon, może być wymagana atrybucja.
- [ ] googletest — tylko testy, nie jest dostarczany w aplikacji → brak działań.

## 📄 Po stronie sklepu (poza kodem)
- [ ] **Polityka prywatności (URL)** — wymagana (dane fitness/zdrowotne).
- [ ] **Formularz Data Safety** — zadeklarować: dane trzymane lokalnie, brak wysyłki off-device przez aplikację, użycie schowka (prompt do LLM kopiowany do clipboard, wklejany ręcznie przez użytkownika).
- [ ] **Listing sklepu** — tytuł, opis, zrzuty ekranu, feature graphic, kategoria.
- [ ] **Content rating** (kwestionariusz) + target audience/wiek.
- [ ] **Konto deweloperskie** (jednorazowo $25), App access (brak logowania), oraz opis w listingu, że plan generuje zewnętrzny LLM przez schowek.
