#pragma once

#include <QString>
#include <memory>

class QmlRegistrator;

class BackendWorker;
class AppDbStorage;
class WorkoutService;
class WorkoutTemplateService;
class UserProfileService;
class ExerciseCatalogService;
class CatalogDefinitionLookup;
class ActiveWorkoutViewModel;
class WorkoutHistoryViewModel;
class PlannedWorkoutViewModel;
class UserProfileViewModel;
class ExerciseCatalogViewModel;
class WorkoutEditorViewModel;
class WorkoutTemplateViewModel;
class ClipboardHelper;

class LiftPlannerApplication final
{
public:
    explicit LiftPlannerApplication(const QString& dbPath);
    ~LiftPlannerApplication();

    bool initialize();
    void registerQmlTypes(QmlRegistrator& registrator);

private:
    void drainWorker();

    std::unique_ptr<BackendWorker> m_worker;
    std::unique_ptr<AppDbStorage> m_storage;
    std::unique_ptr<WorkoutService> m_workoutService;
    std::unique_ptr<UserProfileService> m_userProfileService;
    std::unique_ptr<ExerciseCatalogService> m_exerciseCatalogService;
    std::unique_ptr<CatalogDefinitionLookup> m_definitionLookup;
    std::unique_ptr<WorkoutTemplateService> m_workoutTemplateService;

    std::unique_ptr<ActiveWorkoutViewModel> m_activeWorkoutViewModel;
    std::unique_ptr<WorkoutHistoryViewModel> m_workoutHistoryViewModel;
    std::unique_ptr<PlannedWorkoutViewModel> m_plannedWorkoutViewModel;
    std::unique_ptr<UserProfileViewModel> m_userProfileViewModel;
    std::unique_ptr<ExerciseCatalogViewModel> m_exerciseCatalogViewModel;
    std::unique_ptr<WorkoutEditorViewModel> m_workoutEditorViewModel;
    std::unique_ptr<WorkoutTemplateViewModel> m_workoutTemplateViewModel;
    std::unique_ptr<ClipboardHelper> m_clipboardHelper;
};
