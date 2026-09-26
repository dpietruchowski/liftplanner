#include "testapplication.h"

#include "application/exercisecatalog/exercisecatalogservice.h"
#include "application/userprofile/userprofileservice.h"
#include "application/workout/workoutservice.h"
#include "application/workout/workouttemplateservice.h"
#include "async/backendworker.h"
#include "async/mocktimeprovider.h"
#include "async/timeprovider.h"
#include "infrastructure/exercisecatalog/catalogdefinitionlookup.h"
#include "infrastructure/exercisecatalog/exercisedefinitionrepositorydb.h"
#include "infrastructure/userprofile/userprofilerepositorydb.h"
#include "infrastructure/workout/workoutrepositorydb.h"
#include "infrastructure/workout/workoutrowrepositorydb.h"
#include "infrastructure/workout/workouttemplaterepositorydb.h"
#include "infrastructure/workout/workouttemplaterowrepositorydb.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/exercisecatalogviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"
#include "ui/viewmodels/userprofileviewmodel.h"
#include "ui/viewmodels/workouteditorviewmodel.h"
#include "ui/viewmodels/workouthistoryviewmodel.h"
#include "ui/viewmodels/workouttemplateviewmodel.h"
#include <QMetaObject>
#include <QSqlQuery>
#include <dbtoolkit/dbstorage.h>

int TestApplication::s_connectionCounter = 0;

TestApplication::TestApplication()
{
    TimeProvider::setInstance(std::make_unique<MockTimeProvider>());
    timeProvider().setCurrentDateTime(QDateTime(QDate(2025, 1, 1), QTime(12, 0, 0)));

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
            m_workoutRowRepo = std::make_unique<WorkoutRowRepositoryDb>(*m_dbStorage);
            m_userProfileRepo = std::make_unique<UserProfileRepositoryDb>(*m_dbStorage);
            m_userProfileRepo->createTable();
            m_exerciseDefinitionRepo
                = std::make_unique<ExerciseDefinitionRepositoryDb>(*m_dbStorage);
            m_exerciseDefinitionRepo->createTables();
            m_workoutTemplateRepo = std::make_unique<WorkoutTemplateRepositoryDb>(*m_dbStorage);
            m_workoutTemplateRepo->createTables();
            m_workoutTemplateRowRepo
                = std::make_unique<WorkoutTemplateRowRepositoryDb>(*m_dbStorage);

            m_workoutService = std::make_unique<WorkoutService>(*m_workoutRepo, *m_workoutRowRepo,
                                                                m_worker.get());
            m_userProfileService
                = std::make_unique<UserProfileService>(*m_userProfileRepo, m_worker.get());
            m_exerciseCatalogService = std::make_unique<ExerciseCatalogService>(
                *m_exerciseDefinitionRepo, m_worker.get());
            m_definitionLookup
                = std::make_unique<CatalogDefinitionLookup>(*m_exerciseDefinitionRepo);
            m_workoutTemplateService = std::make_unique<WorkoutTemplateService>(
                *m_workoutTemplateRepo, *m_workoutTemplateRowRepo, *m_definitionLookup,
                m_worker.get());
        },
        Qt::BlockingQueuedConnection);

    m_activeWorkoutViewModel = std::make_unique<ActiveWorkoutViewModel>(m_workoutService.get());
    m_workoutHistoryViewModel = std::make_unique<WorkoutHistoryViewModel>(
        m_workoutService.get(), m_activeWorkoutViewModel.get());
    m_plannedWorkoutViewModel = std::make_unique<PlannedWorkoutViewModel>(
        m_workoutService.get(), m_userProfileService.get(), m_activeWorkoutViewModel.get());
    m_exerciseCatalogViewModel = std::make_unique<ExerciseCatalogViewModel>(
        m_exerciseCatalogService.get(), m_workoutService.get());
    m_workoutEditorViewModel = std::make_unique<WorkoutEditorViewModel>(
        m_workoutService.get(), m_workoutTemplateService.get());
    m_workoutTemplateViewModel
        = std::make_unique<WorkoutTemplateViewModel>(m_workoutTemplateService.get());
    m_userProfileViewModel = std::make_unique<UserProfileViewModel>(m_userProfileService.get());

    drain();
}

TestApplication::~TestApplication()
{
    drain();

    m_userProfileViewModel.reset();
    m_workoutTemplateViewModel.reset();
    m_workoutEditorViewModel.reset();
    m_exerciseCatalogViewModel.reset();
    m_plannedWorkoutViewModel.reset();
    m_workoutHistoryViewModel.reset();
    m_activeWorkoutViewModel.reset();

    QMetaObject::invokeMethod(
        m_worker.get(),
        [this]()
        {
            m_workoutTemplateService.reset();
            m_definitionLookup.reset();
            m_exerciseCatalogService.reset();
            m_workoutService.reset();
            m_userProfileService.reset();
            m_workoutTemplateRowRepo.reset();
            m_workoutTemplateRepo.reset();
            m_exerciseDefinitionRepo.reset();
            m_userProfileRepo.reset();
            m_workoutRowRepo.reset();
            m_workoutRepo.reset();
            m_dbStorage.reset();
            m_database.close();
            m_database = QSqlDatabase();
        },
        Qt::BlockingQueuedConnection);

    QSqlDatabase::removeDatabase(m_connectionName);

    TimeProvider::setInstance(std::make_unique<SystemTimeProvider>());
}

MockTimeProvider& TestApplication::timeProvider()
{
    return dynamic_cast<MockTimeProvider&>(TimeProvider::instance());
}

void TestApplication::setCurrentDate(const QDate& date) { timeProvider().setCurrentDate(date); }

void TestApplication::setCurrentDateTime(const QDateTime& dateTime)
{
    timeProvider().setCurrentDateTime(dateTime);
}

void TestApplication::advanceSeconds(int seconds) { timeProvider().advanceSeconds(seconds); }

void TestApplication::advanceDay() { timeProvider().advanceDays(1); }

void TestApplication::advanceDays(int days) { timeProvider().advanceDays(days); }

void TestApplication::advanceDate(const QDate& targetDate)
{
    timeProvider().advanceDate(targetDate);
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
ExerciseCatalogViewModel& TestApplication::exerciseCatalogViewModel()
{
    return *m_exerciseCatalogViewModel;
}
WorkoutTemplateViewModel& TestApplication::workoutTemplateViewModel()
{
    return *m_workoutTemplateViewModel;
}

UserProfileViewModel& TestApplication::userProfileViewModel() { return *m_userProfileViewModel; }

WorkoutEditorViewModel& TestApplication::workoutEditorViewModel()
{
    return *m_workoutEditorViewModel;
}
WorkoutService& TestApplication::workoutService() { return *m_workoutService; }
WorkoutTemplateService& TestApplication::workoutTemplateService()
{
    return *m_workoutTemplateService;
}
ExerciseCatalogService& TestApplication::exerciseCatalogService()
{
    return *m_exerciseCatalogService;
}

int TestApplication::seedDefinition(const ExerciseDefinition& definition)
{
    int id = -1;
    QMetaObject::invokeMethod(
        m_worker.get(), [this, &definition, &id]()
        { id = m_exerciseDefinitionRepo->save(definition); }, Qt::BlockingQueuedConnection);
    return id;
}

int TestApplication::seedTemplate(const WorkoutTemplate& workoutTemplate)
{
    int id = -1;
    QMetaObject::invokeMethod(
        m_worker.get(), [this, &workoutTemplate, &id]()
        { id = m_workoutTemplateRepo->save(workoutTemplate); }, Qt::BlockingQueuedConnection);
    return id;
}

void TestApplication::drain() { m_worker->drain(); }
