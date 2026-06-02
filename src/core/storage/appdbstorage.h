#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <memory>

class DbStorage;
class WorkoutRepositoryDb;
class UserProfileRepositoryDb;

class AppDbStorage : public QObject
{
    Q_OBJECT

public:
    explicit AppDbStorage(const QString& dbPath, QObject* parent = nullptr);
    ~AppDbStorage() override;

    WorkoutRepositoryDb& workoutRepo();
    UserProfileRepositoryDb& userProfileRepo();

private:
    void initializeDatabase();

    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutRepositoryDb> m_workoutRepo;
    std::unique_ptr<UserProfileRepositoryDb> m_userProfileRepo;
};
