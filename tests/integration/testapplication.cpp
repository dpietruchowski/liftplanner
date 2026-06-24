#include "testapplication.h"

#include "modules/userprofile/application/userprofileservice.h"
#include "modules/userprofile/infrastructure/database/userprofilerepositorydb.h"
#include "modules/workout/application/workoutservice.h"
#include "modules/workout/infrastructure/database/workoutrepositorydb.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"
#include "ui/viewmodels/workouthistoryviewmodel.h"
#include "utils/backendworker.h"
#include <QEventLoop>
#include <QMetaObject>
#include <QSqlQuery>
#include <dbtoolkit/dbstorage.h>

int TestApplication::s_connectionCounter = 0;

TestApplication::TestApplication()
{
    m_worker = std::make_unique<BackendWorker>();
    m_connectionName = QString("integration_test_%1").arg(++s_connectionCounter);

    QMetaObject::invokeMethod(
        m_worker.get(),
        [this]()
        {
            m_database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
            m_database.setDatabaseName(":memory:");
            m_database.open();

            QSqlQuery pragma(m_database);
            pragma.exec("PRAGMA foreign_keys = ON;");

            m_dbStorage = std::make_unique<DbStorage>(m_database);
            m_workoutRepo = std::make_unique<WorkoutRepositoryDb>(*m_dbStorage);
            m_workoutRepo->createTables();
            m_userProfileRepo = std::make_unique<UserProfileRepositoryDb>(*m_dbStorage);
            m_userProfileRepo->createTable();

            m_workoutService = std::make_unique<WorkoutService>(*m_workoutRepo, m_worker.get());
            m_userProfileService
                = std::make_unique<UserProfileService>(*m_userProfileRepo, m_worker.get());
        },
        Qt::BlockingQueuedConnection);

    m_activeWorkoutViewModel = std::make_unique<ActiveWorkoutViewModel>(m_workoutService.get());
    m_workoutHistoryViewModel = std::make_unique<WorkoutHistoryViewModel>(
        m_workoutService.get(), m_activeWorkoutViewModel.get());
    m_plannedWorkoutViewModel = std::make_unique<PlannedWorkoutViewModel>(
        m_workoutService.get(), m_userProfileService.get());

    drain();
}

TestApplication::~TestApplication()
{
    drain();

    m_plannedWorkoutViewModel.reset();
    m_workoutHistoryViewModel.reset();
    m_activeWorkoutViewModel.reset();

    QMetaObject::invokeMethod(
        m_worker.get(),
        [this]()
        {
            m_workoutService.reset();
            m_userProfileService.reset();
            m_userProfileRepo.reset();
            m_workoutRepo.reset();
            m_dbStorage.reset();
            m_database.close();
            m_database = QSqlDatabase();
        },
        Qt::BlockingQueuedConnection);

    QSqlDatabase::removeDatabase(m_connectionName);
}

ActiveWorkoutViewModel& TestApplication::activeWorkoutViewModel()
{
    return *m_activeWorkoutViewModel;
}
WorkoutHistoryViewModel& TestApplication::workoutHistoryViewModel()
{
    return *m_workoutHistoryViewModel;
}
PlannedWorkoutViewModel& TestApplication::plannedWorkoutViewModel()
{
    return *m_plannedWorkoutViewModel;
}
WorkoutService& TestApplication::workoutService() { return *m_workoutService; }

void TestApplication::drain()
{
    QEventLoop loop;
    for (int i = 0; i < 64; ++i)
    {
        QMetaObject::invokeMethod(m_worker.get(), [] {}, Qt::BlockingQueuedConnection);
        if (!loop.processEvents(QEventLoop::AllEvents))
            break;
    }
}
