#include "testapplication.h"

#include <QSqlQuery>
#include <dbtoolkit/dbstorage.h>
#include "modules/workout/application/workoutservice.h"
#include "modules/workout/infrastructure/database/workoutrepositorydb.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/workouthistoryviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"

int TestApplication::s_connectionCounter = 0;

TestApplication::TestApplication()
{
    m_connectionName = QString("integration_test_%1").arg(++s_connectionCounter);

    m_database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_database.setDatabaseName(":memory:");
    m_database.open();

    QSqlQuery pragma(m_database);
    pragma.exec("PRAGMA foreign_keys = ON;");

    m_dbStorage = std::make_unique<DbStorage>(m_database);
    m_workoutRepo = std::make_unique<WorkoutRepositoryDb>(*m_dbStorage);
    m_workoutRepo->createTables();

    m_workoutService = std::make_unique<WorkoutService>(*m_workoutRepo);

    m_activeWorkoutViewModel = std::make_unique<ActiveWorkoutViewModel>(m_workoutService.get());
    m_workoutHistoryViewModel = std::make_unique<WorkoutHistoryViewModel>(m_workoutService.get());
    m_plannedWorkoutViewModel = std::make_unique<PlannedWorkoutViewModel>(m_workoutService.get());
}

TestApplication::~TestApplication()
{
    m_plannedWorkoutViewModel.reset();
    m_workoutHistoryViewModel.reset();
    m_activeWorkoutViewModel.reset();
    m_workoutService.reset();
    m_workoutRepo.reset();
    m_dbStorage.reset();
    m_database.close();
    m_database = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connectionName);
}

ActiveWorkoutViewModel &TestApplication::activeWorkoutViewModel() { return *m_activeWorkoutViewModel; }
WorkoutHistoryViewModel &TestApplication::workoutHistoryViewModel() { return *m_workoutHistoryViewModel; }
PlannedWorkoutViewModel &TestApplication::plannedWorkoutViewModel() { return *m_plannedWorkoutViewModel; }
WorkoutService &TestApplication::workoutService() { return *m_workoutService; }
