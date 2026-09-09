#pragma once
#include <QObject>
#include <QString>
#include <memory>

class QSqlDatabase;
class DbStorage;
class WorkoutRepositoryDb;
class WorkoutRowRepositoryDb;
class WorkoutTemplateRepositoryDb;
class WorkoutTemplateRowRepositoryDb;
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
    WorkoutRowRepositoryDb& workoutRowRepo();
    WorkoutTemplateRepositoryDb& workoutTemplateRepo();
    WorkoutTemplateRowRepositoryDb& workoutTemplateRowRepo();
    UserProfileRepositoryDb& userProfileRepo();
    ExerciseDefinitionRepositoryDb& exerciseDefinitionRepo();

private:
    void seedExerciseCatalog();
    void backfillExerciseHistory();

    std::unique_ptr<QSqlDatabase> m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutRepositoryDb> m_workoutRepo;
    std::unique_ptr<WorkoutRowRepositoryDb> m_workoutRowRepo;
    std::unique_ptr<WorkoutTemplateRepositoryDb> m_workoutTemplateRepo;
    std::unique_ptr<WorkoutTemplateRowRepositoryDb> m_workoutTemplateRowRepo;
    std::unique_ptr<UserProfileRepositoryDb> m_userProfileRepo;
    std::unique_ptr<ExerciseDefinitionRepositoryDb> m_exerciseDefinitionRepo;
};
