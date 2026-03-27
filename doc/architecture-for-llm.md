# Architecture Blueprint — Qt6/QML + Clean Architecture (DDD)

This document describes a project architecture template for a **Qt6 C++/QML desktop application** using **Clean Architecture** with **Domain-Driven Design (DDD)** patterns. It is designed as instructions for an LLM to bootstrap a new Hello World project with this same architecture.

---

## 1. High-Level Overview

```
┌─────────────────────────────────────────────────────────┐
│                      UI Layer                           │
│   QML Views  ←→  ViewModels (QObject)  ←→  Models      │
└────────────────────────┬────────────────────────────────┘
                         │ depends on
┌────────────────────────▼────────────────────────────────┐
│                  Application Layer                      │
│          Services (orchestrate domain logic)            │
│          Return Result<T> for error handling            │
└────────────────────────┬────────────────────────────────┘
                         │ depends on
┌────────────────────────▼────────────────────────────────┐
│                    Domain Layer                         │
│   Entities  │  Repository Interfaces  │  Query Objects  │
│   (pure business logic, no DB concepts)                 │
└────────────────────────▲────────────────────────────────┘
                         │ implements
┌────────────────────────┴────────────────────────────────┐
│                Infrastructure Layer                     │
│   Database Repositories  │  Serializers                 │
│   (SQLite via DbToolkit, QUuid as BLOB)                 │
└─────────────────────────────────────────────────────────┘
```

**Dependency Rule:** UI → Application → Domain ← Infrastructure

The Domain layer has **zero** dependencies on infrastructure or UI. Infrastructure implements domain interfaces (Dependency Inversion).

---

## 2. Project Directory Structure

```
project-root/
├── CMakeLists.txt                    # Root CMake — project definition
├── .gitmodules                       # Git submodules (libs, googletest)
├── libs/                             # Git submodule — shared reusable libraries
│   ├── CMakeLists.txt
│   ├── dbtoolkit/                    # Database abstraction layer (SHARED lib)
│   │   ├── dbstorage.h/.cpp         # QSqlDatabase wrapper with mutex
│   │   ├── dbrepository.h/.cpp      # Generic CRUD operations on QVariantMap
│   │   ├── dbtoolkit.h              # Convenience header (includes all)
│   │   └── query/                   # SQL query builder classes
│   │       ├── where.h/.cpp         # Fluent WHERE clause builder
│   │       ├── order.h/.cpp         # Fluent ORDER BY builder
│   │       ├── select.h/.cpp        # SELECT query builder
│   │       ├── insert.h/.cpp        # INSERT builder
│   │       ├── update.h/.cpp        # UPDATE builder
│   │       ├── delete.h/.cpp        # DELETE builder
│   │       ├── createtable.h/.cpp   # CREATE TABLE builder
│   │       ├── column.h/.cpp        # Column definition builder
│   │       ├── join.h/.cpp          # JOIN builder
│   │       └── alias.h/.cpp         # Table alias support
│   ├── utils/                        # App utilities (STATIC lib)
│   │   ├── qmlregistrator.h/.cpp    # QML type registration helper
│   │   ├── timeprovider.h/.cpp      # Singleton time provider (testable)
│   │   └── mocktimeprovider.h/.cpp  # Mock for unit tests
│   ├── qmlcomponents/               # Reusable themed QML components (QML module)
│   └── icons/                       # SVG icons as Qt resources
├── src/
│   ├── CMakeLists.txt               # Links all modules, creates executable
│   ├── main.cpp                     # Entry point
│   ├── core/                        # Core infrastructure
│   │   ├── CMakeLists.txt
│   │   ├── types.h                  # Shared enums (SortDirection, etc.)
│   │   ├── app/                     # Application bootstrap
│   │   │   ├── application.h/.cpp   # Owns Storage, Services, ViewModels
│   │   ├── storage/                 # Database orchestration
│   │   │   ├── appstorage.h/.cpp    # Creates all DB repos, manages connection
│   │   ├── events/                  # Event system
│   │   │   ├── eventbase.h/.cpp     # Base event with EventType enum
│   │   │   └── eventbus.h/.cpp      # Pub/sub event bus
│   │   └── user/                    # Cross-cutting user context
│   │       └── application/
│   │           └── usercontext.h/.cpp
│   ├── modules/                     # Feature modules (DDD bounded contexts)
│   │   └── <module_name>/
│   │       ├── CMakeLists.txt
│   │       ├── domain/
│   │       │   ├── entities/        # Business objects
│   │       │   └── repositories/    # Interfaces + Query objects
│   │       ├── application/         # Services
│   │       └── infrastructure/
│   │           ├── database/        # SQLite repository implementations
│   │           └── serializers/     # Entity ↔ QVariantMap converters
│   └── ui/
│       ├── viewmodels/              # QObject-based ViewModels for QML
│       ├── models/                  # QAbstractListModel-based list models
│       └── qml/                     # QML files (Qt QML module)
└── tests/
    ├── CMakeLists.txt
    ├── third_party/googletest/      # Git submodule
    └── <module_name>/               # Mirror of module structure
        ├── domain/entities/         # Entity unit tests
        ├── application/             # Service tests (with mocks)
        └── infrastructure/database/ # Repository integration tests
```

---

## 3. Root CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)

project(myapp VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libs)
add_subdirectory(src)

if(NOT ANDROID AND BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
```

---

## 4. Libs Submodule

The `libs/` directory is a **git submodule** — a separate repository shared across projects. It contains reusable, project-agnostic libraries.

### 4.1 DbToolkit (SHARED library)

The core database abstraction layer. Built as a **shared library** (`add_library(dbtoolkit SHARED ...)`).

**DbStorage** — wraps `QSqlDatabase` with thread-safe mutex:
```cpp
class DbStorage {
public:
    explicit DbStorage(QSqlDatabase& database);
    QVector<QVariantMap> execute(const Select& query);
    QVariant execute(const SqlCommand& command);
    bool beginTransaction();
    bool commit();
    bool rollback();
};
```

**DbRepository** — generic CRUD on a table using `QVariantMap`:
```cpp
class DbRepository : public QObject {
public:
    DbRepository(const QString& tableName, const QString& idKey,
                 const QStringList& keys, DbStorage& storage, QObject* parent = nullptr);
    
    bool createTable(const CreateTable& tableDefinition);
    QVector<QVariantMap> select(const Where& condition = Where(), const Order& order = Order(),
                                int limit = -1, int offset = -1) const;
    QVariant insert(const QVariantMap& item);
    int update(const QVariantMap& item, const Where& condition = Where());
    int remove(const Where& condition);
    bool exists(const Where& condition) const;
    int count(const Where& condition = {}) const;
};
```

**Query Builder** — fluent API for SQL construction:
```cpp
// WHERE clause
Where("date").equals("2024-01-01").and_("status").equals(1)

// ORDER BY
Order("date").desc().then("name").asc()

// CREATE TABLE
CreateTable("items")
    .ifNotExists()
    .column(Column("uuid").blob().primaryKey().notNull())
    .column(Column("name").text().notNull())
    .column(Column("value").integer().defaultValue(0))
    .uniqueConstraint({"name", "category"});
```

### 4.2 Utils (STATIC library)

Target name: `app_utils`

- **QmlRegistrator** — helper to register ViewModels and types with QML engine
- **TimeProvider** — singleton with `currentDateTime()`, replaceable with `MockTimeProvider` for tests

### 4.3 QML Components (QML module)

URI: `Themed.Components` — reusable themed UI components (ThemedButton, ThemedCard, ThemedInput, etc.)

### 4.4 Icons

SVG icons bundled as Qt resources under `/Themed/Icons/` prefix.

### 4.5 Libs CMakeLists.txt

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml Sql)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(dbtoolkit)
add_subdirectory(utils)
add_subdirectory(icons)
add_subdirectory(qmlcomponents)
```

---

## 5. Module Structure (DDD Bounded Contexts)

Each feature module follows the same 3-layer structure:

```
src/modules/<module_name>/
├── CMakeLists.txt
├── domain/
│   ├── entities/          # Pure business objects
│   └── repositories/      # Repository interface + Query object
├── application/           # Service(s)
└── infrastructure/
    ├── database/          # Repository DB implementation
    └── serializers/       # Entity ↔ QVariantMap
```

### 5.1 Module CMakeLists.txt Pattern

```cmake
set(LIBRARY_NAME <module>_module)

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")

add_library(${LIBRARY_NAME} STATIC ${SOURCES} ${HEADERS})

target_include_directories(${LIBRARY_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}
)

target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        core
        dbtoolkit
        Qt6::Core
        Qt6::Sql
)

target_compile_options(${LIBRARY_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)
```

---

## 6. Domain Layer

### 6.1 Entity

Entities are `final` classes with:
- Constructor with all fields
- Getters/setters
- Private `validate()` method
- Static factory/business logic methods
- `m_` prefixed members
- **No database concepts** (no UUID — that's infrastructure)
- Only Qt Core types allowed (QString, QDate, etc.)

```cpp
#pragma once

#include <QString>

class Item final
{
public:
    Item();
    Item(const QString& name, int value);

    const QString& name() const;
    int value() const;

    void setName(const QString& name);
    void setValue(int value);

    static Item createDefault(const QString& name);

private:
    void validate() const;

    QString m_name;
    int m_value { 0 };
};
```

### 6.2 Repository Interface

Pure virtual interface in domain layer. Standard method signatures:

```cpp
#pragma once

#include <optional>
#include <vector>

class Item;
class ItemQuery;

class ItemRepository
{
public:
    virtual ~ItemRepository() = default;

    virtual std::vector<Item> findAll(const ItemQuery& query) const = 0;
    virtual std::optional<Item> findOne(const ItemQuery& query) const = 0;
    virtual bool save(const Item& item) = 0;
    virtual bool remove(const ItemQuery& query) = 0;
    virtual int count(const ItemQuery& query) const = 0;
    virtual bool exists(const ItemQuery& query) const = 0;
};
```

### 6.3 Query Object

Fluent API query object, all fields are `std::optional`:

```cpp
#pragma once

#include "core/types.h"
#include <QString>
#include <optional>

class ItemQuery final
{
public:
    ItemQuery& whereName(const QString& name);
    ItemQuery& orderByName(SortDirection direction);
    ItemQuery& withLimit(int limit);

    const std::optional<QString>& name() const;
    const std::optional<SortDirection>& orderByNameOrder() const;
    const std::optional<int>& limit() const;

private:
    std::optional<QString> m_name;
    std::optional<SortDirection> m_orderByNameOrder;
    std::optional<int> m_limit;
};
```

---

## 7. Application Layer (Services)

Services **orchestrate** domain logic — they don't contain business logic themselves. They:
- Take repository references and `UserContext` in constructor
- Return `Result<T>` for all operations
- Use try/catch wrapping

```cpp
#pragma once

#include <vector>
#include "modules/item/domain/entities/item.h"
#include "modules/item/domain/repositories/itemrepository.h"
#include "core/result.h"

class UserContext;

class ItemService final
{
public:
    explicit ItemService(ItemRepository& repository, const UserContext& userContext);

    Result<Item> getItem(const QString& name);
    Result<std::vector<Item>> getAllItems();
    Result<bool> saveItem(const Item& item);

private:
    ItemRepository& m_repository;
    const UserContext& m_userContext;
};
```

### 7.1 Result<T> Type

Generic result wrapper for service return values:

```cpp
#pragma once

#include <QString>
#include <optional>

template<typename T>
class Result final
{
public:
    static Result success(const T& value) { return Result{value, std::nullopt}; }
    static Result failure(const QString& error) { return Result{std::nullopt, error}; }

    bool isSuccess() const { return m_value.has_value(); }
    bool isFailure() const { return !isSuccess(); }
    const T& value() const { return m_value.value(); }
    const QString& error() const { return m_error.value(); }

private:
    Result(std::optional<T> value, std::optional<QString> error)
        : m_value(std::move(value)), m_error(std::move(error)) {}

    std::optional<T> m_value;
    std::optional<QString> m_error;
};

template<>
class Result<void> final
{
public:
    static Result success() { return Result{std::nullopt}; }
    static Result failure(const QString& error) { return Result{error}; }

    bool isSuccess() const { return !m_error.has_value(); }
    bool isFailure() const { return !isSuccess(); }
    const QString& error() const { return m_error.value(); }

private:
    explicit Result(std::optional<QString> error) : m_error(std::move(error)) {}
    std::optional<QString> m_error;
};
```

---

## 8. Infrastructure Layer

### 8.1 Serializer

Converts between domain entity and `QVariantMap` (database row representation). Also defines table/column name constants.

```cpp
#pragma once

#include <QVariantMap>

class Item;

class ItemSerializer
{
public:
    static constexpr const char* item_table = "items";
    static constexpr const char* uuid_key = "uuid";
    static constexpr const char* name_key = "name";
    static constexpr const char* value_key = "value";

    static Item fromVariant(const QVariantMap& data);
    QVariantMap toVariant(const Item& item) const;
};
```

### 8.2 Database Repository

Implements the domain repository interface. Uses `DbRepository` from dbtoolkit.

```cpp
#pragma once

#include <memory>
#include <vector>
#include "modules/item/domain/repositories/itemrepository.h"
#include "modules/item/infrastructure/serializers/itemserializer.h"

class DbStorage;
class DbRepository;
class Where;
class Order;

class ItemRepositoryDb : public ItemRepository
{
public:
    explicit ItemRepositoryDb(DbStorage& storage);
    ~ItemRepositoryDb() override;

    bool createTable();

    std::vector<Item> findAll(const ItemQuery& query) const override;
    std::optional<Item> findOne(const ItemQuery& query) const override;
    bool save(const Item& item) override;
    bool remove(const ItemQuery& query) override;
    int count(const ItemQuery& query) const override;
    bool exists(const ItemQuery& query) const override;

private:
    std::unique_ptr<DbRepository> m_repository;
    ItemSerializer m_serializer;

    Where buildWhereClause(const ItemQuery& query) const;
    Order buildOrderClause(const ItemQuery& query) const;
};
```

**Implementation pattern** (constructor creates DbRepository, methods use serializer):
```cpp
ItemRepositoryDb::ItemRepositoryDb(DbStorage& storage)
    : m_repository(std::make_unique<DbRepository>(
          ItemSerializer::item_table, ItemSerializer::uuid_key,
          QStringList { ItemSerializer::uuid_key, ItemSerializer::name_key,
                        ItemSerializer::value_key },
          storage, nullptr))
{}

bool ItemRepositoryDb::createTable()
{
    CreateTable table(ItemSerializer::item_table);
    table.ifNotExists()
        .column(Column(ItemSerializer::uuid_key).blob().primaryKey().notNull())
        .column(Column(ItemSerializer::name_key).text().notNull())
        .column(Column(ItemSerializer::value_key).integer().defaultValue(0));
    return m_repository->createTable(table);
}

bool ItemRepositoryDb::save(const Item& item)
{
    QVariantMap data = m_serializer.toVariant(item);
    auto query = ItemQuery().whereName(item.name());
    if (exists(query))
    {
        Where where = buildWhereClause(query);
        return m_repository->update(data, where) > 0;
    }
    else
    {
        data[ItemSerializer::uuid_key] = QUuid::createUuid().toRfc4122();
        return m_repository->insert(data).isValid();
    }
}
```

**UUID as BLOB:** UUIDs are stored as 16-byte BLOBs via `QUuid::createUuid().toRfc4122()`. Domain entities never see UUIDs — they are an infrastructure concern.

---

## 9. Core Layer

### 9.1 Application Bootstrap Class

Central class that owns the entire object graph. Created in `main.cpp`.

```cpp
class MyApplication final
{
public:
    explicit MyApplication(const QString& databasePath);
    ~MyApplication();

    bool initialize();
    void registerQmlTypes(QmlRegistrator& registrator);

private:
    void createServices();
    void createViewModels();

    std::unique_ptr<AppStorage> m_storage;
    std::unique_ptr<EventBus> m_eventBus;
    std::unique_ptr<UserContext> m_userContext;

    // Services
    std::unique_ptr<ItemService> m_itemService;

    // ViewModels
    std::unique_ptr<ItemViewModel> m_itemViewModel;
};
```

**initialize()** flow:
1. Open database (`m_storage->open()`)
2. Create tables (`m_storage->createTables()`)
3. Create `UserContext` from settings repository
4. Create `EventBus`
5. `createServices()` — instantiate all services with repository refs
6. `createViewModels()` — instantiate ViewModels, inject services

**registerQmlTypes()** — register ViewModels as QML singletons:
```cpp
void MyApplication::registerQmlTypes(QmlRegistrator& registrator)
{
    registrator.registerSingletonInstance("ItemViewModel", m_itemViewModel.get());
}
```

### 9.2 AppStorage

Owns `QSqlDatabase`, `DbStorage`, and all `*RepositoryDb` instances. Acts as repository factory.

```cpp
class AppStorage final
{
public:
    explicit AppStorage(const QString& databasePath);
    ~AppStorage();

    bool open();
    void createTables();

    DbStorage& storage();
    ItemRepositoryDb& itemRepository();

private:
    std::unique_ptr<QSqlDatabase> m_database;
    std::unique_ptr<DbStorage> m_storage;
    std::unique_ptr<ItemRepositoryDb> m_itemRepo;
};
```

### 9.3 EventBus

Simple publish/subscribe for cross-module communication:

```cpp
enum class EventType { ItemCreated, ItemDeleted };

class EventBase {
public:
    explicit EventBase(EventType type);
    virtual ~EventBase() = default;
    EventType type() const;
private:
    EventType m_type;
    QUuid m_id;
};

class EventBus final {
public:
    void publish(std::shared_ptr<EventBase> event);
    void subscribe(EventType type, std::function<void(const EventBase&)> handler);
private:
    std::unordered_map<EventType, std::vector<std::function<void(const EventBase&)>>> m_handlers;
};
```

### 9.4 UserContext

Cross-cutting concern that holds user settings, injected into services:

```cpp
class UserContext final
{
public:
    explicit UserContext(UserSettingsRepository& repository);
    const UserSettings& settings() const;
    void setSettings(const UserSettings& settings);
    bool saveSettings();
private:
    UserSettings m_settings;
    UserSettingsRepository* m_repository { nullptr };
};
```

### 9.5 Shared Types

```cpp
// core/types.h
#pragma once

enum class SortDirection { Ascending, Descending };
```

---

## 10. UI Layer

### 10.1 ViewModel

QObject-based, exposes data via Q_PROPERTY and actions via Q_INVOKABLE:

```cpp
class ItemViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString itemName READ itemName NOTIFY itemChanged)
    Q_PROPERTY(int itemValue READ itemValue NOTIFY itemChanged)

public:
    explicit ItemViewModel(QObject* parent = nullptr);

    void setServices(ItemService* itemService);

    QString itemName() const;
    int itemValue() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void saveItem(const QString& name, int value);

signals:
    void itemChanged();

private:
    ItemService* m_itemService = nullptr;
    QString m_itemName;
    int m_itemValue = 0;
};
```

### 10.2 QML Module

QML files are packaged as a Qt QML module:

```cmake
qt_add_qml_module(app_qml
    URI MyApp
    VERSION 1.0
    QML_FILES ${QML_FILES}
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/MyApp
    NO_PLUGIN
)
```

### 10.3 main.cpp

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "core/app/myapplication.h"
#include "utils/qmlregistrator.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    MyApplication myApp("myapp.db");
    if (!myApp.initialize())
        return 1;

    QQmlApplicationEngine engine;

    QString uiRootDir = "qrc:/MyApp/";
    QmlRegistrator registrator(engine, uiRootDir, "MyApp");

    myApp.registerQmlTypes(registrator);

    engine.load(registrator.getMainQmlUrl());

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
```

---

## 11. Tests

Uses **Google Test + Google Mock** (git submodule in `tests/third_party/googletest/`).

### 11.1 Test Structure

Tests mirror module structure:
```
tests/<module_name>/
├── CMakeLists.txt
├── domain/entities/          # Unit tests for entities
├── application/              # Service tests with mocked repositories
└── infrastructure/database/  # Integration tests with real SQLite
```

### 11.2 Test CMakeLists.txt Pattern

```cmake
set(TEST_SOURCES
    application/itemservicetest.cpp
    domain/entities/itemtest.cpp
    infrastructure/database/itemrepositorydb_test.cpp
)

add_executable(item_tests ${TEST_SOURCES})

target_compile_definitions(item_tests PRIVATE UNIT_TEST)

target_link_libraries(item_tests
    PRIVATE
        core
        item_module
        gtest
        gtest_main
        gmock
        Qt6::Core
        Qt6::Sql
)

target_include_directories(item_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}
)

include(GoogleTest)
gtest_discover_tests(item_tests)
```

### 11.3 Service Test Pattern (with Mocks)

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "modules/item/domain/repositories/itemrepository.h"

class MockItemRepository : public ItemRepository
{
public:
    MOCK_METHOD(std::vector<Item>, findAll, (const ItemQuery& query), (const, override));
    MOCK_METHOD(std::optional<Item>, findOne, (const ItemQuery& query), (const, override));
    MOCK_METHOD(bool, save, (const Item& item), (override));
    MOCK_METHOD(bool, remove, (const ItemQuery& query), (override));
    MOCK_METHOD(int, count, (const ItemQuery& query), (const, override));
    MOCK_METHOD(bool, exists, (const ItemQuery& query), (const, override));
};

class ItemServiceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockRepo = std::make_unique<NiceMock<MockItemRepository>>();
        userContext = std::make_unique<UserContext>(UserSettings());
        service = std::make_unique<ItemService>(*mockRepo, *userContext);
    }

    std::unique_ptr<NiceMock<MockItemRepository>> mockRepo;
    std::unique_ptr<UserContext> userContext;
    std::unique_ptr<ItemService> service;
};

TEST_F(ItemServiceTest, GetItem_ReturnsItem_WhenExists)
{
    Item expected("test", 42);
    ON_CALL(*mockRepo, findOne(_)).WillByDefault(Return(std::optional(expected)));

    auto result = service->getItem("test");

    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value().name(), "test");
}
```

---

## 12. Coding Conventions

| Rule | Example |
|------|---------|
| Classes | `PascalCase final` — `class Item final` |
| Members | `m_` prefix — `m_repository` |
| Methods | `camelCase` — `calculateStreak()` |
| Enums | `enum class` — `enum class SortDirection { Ascending, Descending }` |
| Files | Match class name exactly — `ItemRepository.h` |
| Headers | `#pragma once` |
| No comments | Code must be self-documenting |
| Constructors | Member initializer lists |
| Entities | Private `validate()` method |
| C++17/20 | `std::optional`, `std::unique_ptr`, `auto` where type is obvious |
| Query calls | Single line: `auto r = repo.findAll(ItemQuery().whereName(n).withLimit(1));` |

---

## 13. Build System

### Dependencies
- **Qt6**: Core, Gui, Qml, Quick, Sql
- **C++20** standard
- **CMake 3.16+**
- **SQLite** (via Qt6::Sql)
- **Google Test/Mock** (git submodule)

### Build Commands
```bash
# Configure
cmake -S . -B build/Debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
cmake --build build/Debug --config Debug --parallel

# Run tests
ctest --output-on-failure --test-dir build/Debug
```

### Key CMake Features
- `CMAKE_AUTOMOC ON` for Qt meta-object compiler
- `CMAKE_AUTORCC ON` for Qt resource compiler
- `qt_add_qml_module()` for QML modules
- `file(GLOB_RECURSE ...)` for source discovery in modules
- Strict compiler warnings: `-Wall -Wextra -Wpedantic -Werror`

---

## 14. Data Flow Summary

```
User Action (QML)
    → Q_INVOKABLE on ViewModel
        → Service.method()
            → Repository.findAll(Query)  // domain interface
                → RepositoryDb.findAll() // infrastructure impl
                    → DbRepository.select(Where, Order, limit)
                        → DbStorage.execute(Select)
                            → QSqlDatabase
                    ← QVector<QVariantMap>
                ← Serializer::fromVariant() → Entity
            ← Result<Entity>
        ← update Q_PROPERTY, emit signal
    ← QML binding updates UI
```

---

## 15. Git Submodules

The project uses two git submodules:

```gitmodules
[submodule "libs"]
    path = libs
    url = <your-libs-repo-url>

[submodule "tests/third_party/googletest"]
    path = tests/third_party/googletest
    url = https://github.com/google/googletest.git
```

After cloning, initialize submodules:
```bash
git submodule update --init --recursive
```

The `libs` submodule is a shared library repository reused across multiple projects. It provides:
- **dbtoolkit** — database abstraction with query builder
- **utils** — QML registration, time provider, test support
- **qmlcomponents** — themed QML UI components
- **icons** — SVG icon resources

This allows multiple applications to share the same infrastructure code without duplication.
