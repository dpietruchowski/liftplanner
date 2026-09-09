#pragma once

#include <QDate>
#include <QDateTime>
#include <QSqlDatabase>
#include <memory>

class MockTimeProvider;
class BackendWorker;
class DbStorage;
class WorkoutRepositoryDb;
class WorkoutRowRepositoryDb;
class WorkoutService;
class WorkoutTemplateRepositoryDb;
class WorkoutTemplateRowRepositoryDb;
class WorkoutTemplateService;
class UserProfileRepositoryDb;
class UserProfileService;
class ExerciseDefinition;
class WorkoutTemplate;
class ExerciseDefinitionRepositoryDb;
class ExerciseCatalogService;
class CatalogDefinitionLookup;
class ActiveWorkoutViewModel;
class WorkoutHistoryViewModel;
class PlannedWorkoutViewModel;
class ExerciseCatalogViewModel;
class WorkoutEditorViewModel;
class WorkoutTemplateViewModel;
class UserProfileViewModel;

class TestApplication final
{
public:
    TestApplication();
    ~TestApplication();

    ActiveWorkoutViewModel& activeWorkoutViewModel();
    WorkoutHistoryViewModel& workoutHistoryViewModel();
    PlannedWorkoutViewModel& plannedWorkoutViewModel();
    ExerciseCatalogViewModel& exerciseCatalogViewModel();
    WorkoutEditorViewModel& workoutEditorViewModel();
    WorkoutTemplateViewModel& workoutTemplateViewModel();
    UserProfileViewModel& userProfileViewModel();

    WorkoutService& workoutService();
    WorkoutTemplateService& workoutTemplateService();
    ExerciseCatalogService& exerciseCatalogService();

    int seedDefinition(const ExerciseDefinition& definition);
    int seedTemplate(const WorkoutTemplate& workoutTemplate);

    MockTimeProvider& timeProvider();
    void setCurrentDate(const QDate& date);
    void setCurrentDateTime(const QDateTime& dateTime);
    void advanceSeconds(int seconds);
    void advanceDay();
    void advanceDays(int days);
    void advanceDate(const QDate& targetDate);

    void drain();

private:
    static int s_connectionCounter;

    std::unique_ptr<BackendWorker> m_worker;
    QString m_connectionName;
    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutRepositoryDb> m_workoutRepo;
    std::unique_ptr<WorkoutRowRepositoryDb> m_workoutRowRepo;
    std::unique_ptr<WorkoutService> m_workoutService;
    std::unique_ptr<UserProfileRepositoryDb> m_userProfileRepo;
    std::unique_ptr<UserProfileService> m_userProfileService;
    std::unique_ptr<ExerciseDefinitionRepositoryDb> m_exerciseDefinitionRepo;
    std::unique_ptr<ExerciseCatalogService> m_exerciseCatalogService;
    std::unique_ptr<CatalogDefinitionLookup> m_definitionLookup;
    std::unique_ptr<WorkoutTemplateRepositoryDb> m_workoutTemplateRepo;
    std::unique_ptr<WorkoutTemplateRowRepositoryDb> m_workoutTemplateRowRepo;
    std::unique_ptr<WorkoutTemplateService> m_workoutTemplateService;

    std::unique_ptr<ActiveWorkoutViewModel> m_activeWorkoutViewModel;
    std::unique_ptr<WorkoutHistoryViewModel> m_workoutHistoryViewModel;
    std::unique_ptr<PlannedWorkoutViewModel> m_plannedWorkoutViewModel;
    std::unique_ptr<ExerciseCatalogViewModel> m_exerciseCatalogViewModel;
    std::unique_ptr<WorkoutEditorViewModel> m_workoutEditorViewModel;
    std::unique_ptr<WorkoutTemplateViewModel> m_workoutTemplateViewModel;
    std::unique_ptr<UserProfileViewModel> m_userProfileViewModel;
};
