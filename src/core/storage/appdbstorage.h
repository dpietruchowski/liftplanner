#pragma once
#include <QObject>
#include <QString>
#include <memory>

class QSqlDatabase;
class DbStorage;
class WorkoutRepositoryDb;
class UserProfileRepositoryDb;
class ExerciseDefinitionRepositoryDb;

class AppDbStorage : public QObject
{
    Q_OBJECT

public:
    explicit AppDbStorage(const QString& dbPath, QObject* parent = nullptr);
    ~AppDbStorage() override;

    bool open();

    WorkoutRepositoryDb& workoutRepo();
    UserProfileRepositoryDb& userProfileRepo();
    ExerciseDefinitionRepositoryDb& exerciseDefinitionRepo();

private:
    std::unique_ptr<QSqlDatabase> m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutRepositoryDb> m_workoutRepo;
    std::unique_ptr<UserProfileRepositoryDb> m_userProfileRepo;
    std::unique_ptr<ExerciseDefinitionRepositoryDb> m_exerciseDefinitionRepo;
};
