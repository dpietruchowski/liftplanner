#pragma once

#include "ui/models/setmodel.h"
#include "ui/models/workoutmodel.h"
#include "ui/presentation/serializationutils.h"
#include <QClipboard>
#include <QDateTime>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QVariantList>

class WorkoutService;
class ActiveWorkoutViewModel;

class WorkoutHistoryViewModel : public QObject
{
    Q_OBJECT

    DECLARE_PROPERTY(QList<WorkoutModel*>, workouts, setWorkouts)
    Q_PROPERTY(WorkoutModel* lastWorkout READ lastWorkout NOTIFY lastWorkoutChanged)
    Q_PROPERTY(QVariantList topExercises READ topExercises NOTIFY topExercisesChanged)
    Q_PROPERTY(QVariantList weekActivity READ weekActivity NOTIFY weekActivityChanged)
    Q_PROPERTY(QVariantList recentTotals READ recentTotals NOTIFY recentTotalsChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)

public:
    explicit WorkoutHistoryViewModel(WorkoutService* service,
                                     ActiveWorkoutViewModel* activeWorkoutViewModel = nullptr,
                                     QObject* parent = nullptr);

    Q_INVOKABLE void loadAllWorkouts();
    Q_INVOKABLE void saveWorkout(WorkoutModel* workout);
    Q_INVOKABLE void deleteWorkout(WorkoutModel* workout);
    Q_INVOKABLE void requestSetToggle(SetModel* set);
    Q_INVOKABLE void confirmPendingToggle();
    Q_INVOKABLE void cancelPendingToggle();

    Q_INVOKABLE void importFromJson(const QString& jsonData);
    Q_INVOKABLE void importFromClipboard();
    Q_INVOKABLE void exportToClipboard(int limit = 50);
    Q_INVOKABLE void exportWorkoutToClipboard(WorkoutModel* workout);

    Q_INVOKABLE QJsonArray recentWorkoutsToJson(int count = 10);

    WorkoutModel* lastWorkout() const;
    bool isLoading() const;
    QVariantList topExercises() const;
    QVariantList recentTotals() const;
    QVariantList weekActivity() const;

signals:
    void amnestyWarningRaised(const QString& message);
    void errorOccurred(const QString& errorMessage);
    void loadingChanged();
    void lastWorkoutChanged();
    void topExercisesChanged();
    void recentTotalsChanged();
    void weekActivityChanged();
    void exportedToClipboard();

private:
    WorkoutModel* ownerOf(SetModel* set) const;
    void applyToggle(SetModel* set);
    void setLoading(bool value);
    void refreshTopExercises();
    void refreshRecentTotals();

    WorkoutService* m_service;
    ActiveWorkoutViewModel* m_activeWorkoutViewModel;
    QVariantList m_topExercises;
    QVariantList m_recentTotals;
    QPointer<SetModel> m_pendingToggle;
    bool m_loading { false };
};
