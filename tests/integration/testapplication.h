#pragma once

#include <memory>
#include <QSqlDatabase>

class DbStorage;
class WorkoutRepositoryDb;
class WorkoutService;
class UserProfileRepositoryDb;
class UserProfileService;
class ActiveWorkoutViewModel;
class WorkoutHistoryViewModel;
class PlannedWorkoutViewModel;

class TestApplication final
{
public:
    TestApplication();
    ~TestApplication();

    ActiveWorkoutViewModel &activeWorkoutViewModel();
    WorkoutHistoryViewModel &workoutHistoryViewModel();
    PlannedWorkoutViewModel &plannedWorkoutViewModel();

    WorkoutService &workoutService();

private:
    static int s_connectionCounter;

    QString m_connectionName;
    QSqlDatabase m_database;
    std::unique_ptr<DbStorage> m_dbStorage;
    std::unique_ptr<WorkoutRepositoryDb> m_workoutRepo;
    std::unique_ptr<WorkoutService> m_workoutService;
    std::unique_ptr<UserProfileRepositoryDb> m_userProfileRepo;
    std::unique_ptr<UserProfileService> m_userProfileService;

    std::unique_ptr<ActiveWorkoutViewModel> m_activeWorkoutViewModel;
    std::unique_ptr<WorkoutHistoryViewModel> m_workoutHistoryViewModel;
    std::unique_ptr<PlannedWorkoutViewModel> m_plannedWorkoutViewModel;
};
