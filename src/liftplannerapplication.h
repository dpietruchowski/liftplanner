#pragma once

#include <QString>
#include <memory>

class QmlRegistrator;

class AppDbStorage;
class WorkoutService;
class RoutineViewModel;
class ActiveWorkoutViewModel;
class WorkoutHistoryViewModel;
class ClipboardHelper;

class LiftPlannerApplication final
{
public:
    explicit LiftPlannerApplication(const QString &dbPath);
    ~LiftPlannerApplication();

    void initialize();
    void registerQmlTypes(QmlRegistrator &registrator);

private:
    std::unique_ptr<AppDbStorage> m_storage;
    std::unique_ptr<WorkoutService> m_workoutService;

    std::unique_ptr<RoutineViewModel> m_routineViewModel;
    std::unique_ptr<ActiveWorkoutViewModel> m_activeWorkoutViewModel;
    std::unique_ptr<WorkoutHistoryViewModel> m_workoutHistoryViewModel;
    std::unique_ptr<ClipboardHelper> m_clipboardHelper;
};
