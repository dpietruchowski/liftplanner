#pragma once

#include "ui/models/workoutmodel.h"
#include <QList>
#include <QObject>
#include <QStringList>

class WorkoutService;
class UserProfileService;
class ActiveWorkoutViewModel;

class PlannedWorkoutViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<WorkoutModel*> workouts READ workouts NOTIFY workoutsChanged)
    Q_PROPERTY(WorkoutModel* nextWorkout READ nextWorkout NOTIFY workoutsChanged)
    Q_PROPERTY(WorkoutModel* blankWorkout READ blankWorkout NOTIFY blankWorkoutChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)

public:
    explicit PlannedWorkoutViewModel(WorkoutService* service, UserProfileService* profileService,
                                     ActiveWorkoutViewModel* activeWorkoutViewModel = nullptr,
                                     QObject* parent = nullptr);
    ~PlannedWorkoutViewModel();

    QList<WorkoutModel*> workouts() const;
    WorkoutModel* nextWorkout() const;
    WorkoutModel* blankWorkout() const;
    bool isLoading() const;

    Q_INVOKABLE void loadAll();
    Q_INVOKABLE void repeatWorkout(WorkoutModel* workout);
    Q_INVOKABLE void deleteWorkout(WorkoutModel* workout);
    Q_INVOKABLE void importFromClipboard();
    Q_INVOKABLE void importFromJson(const QString& jsonData);
    Q_INVOKABLE void generatePrompt();

signals:
    void workoutsChanged();
    void blankWorkoutChanged();
    void loadingChanged();
    void errorOccurred(const QString& errorMessage);
    void promptGenerated();
    void workoutRepeated(const QString& name);

private:
    void setLoading(bool value);
    void refreshBlankWorkout();
    bool validateJson(const QString& jsonData, QString& errorMessage);
    static QString summarizeErrors(const QStringList& errors);
    static QString readTemplateFile(const QString& filePath);

    WorkoutService* m_service;
    UserProfileService* m_profileService;
    QList<WorkoutModel*> m_workouts;
    WorkoutModel* m_blankWorkout { nullptr };
    bool m_loading { false };
};
