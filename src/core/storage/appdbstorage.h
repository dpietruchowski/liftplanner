#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <memory>

class DbStorage;
class WorkoutRepositoryDb;

class AppDbStorage : public QObject
{
    Q_OBJECT

public:
    explicit AppDbStorage(const QString &dbPath, QObject *parent = nullptr);
    ~AppDbStorage() override;

    WorkoutRepositoryDb &workoutRepo();

private:
    void initializeDatabase();

    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutRepositoryDb> m_workoutRepo;
};
