#pragma once
#include <QObject>
#include <QList>
#include "../models/workoutmodel.h"
#include "../storage/appdbstorage.h"

class RoutineService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QObject*> workouts READ workouts NOTIFY workoutsChanged)

public:
    explicit RoutineService(AppDbStorage *dbStorage, QObject *parent = nullptr);
    ~RoutineService();

    QList<QObject*> workouts() const;

    Q_INVOKABLE void loadAllWorkouts();

    Q_INVOKABLE void importWorkoutsFromJson(const QString &jsonData);
    Q_INVOKABLE void generateGptPrompt(const QJsonArray &recentTrainings);
    Q_INVOKABLE void importWorkoutsFromClipboard();

signals:
    void workoutsChanged();
    void errorOccurred(const QString &errorMessage);
    void gptPromptGenerated();

private:
    bool validateWorkoutsJson(const QString &jsonData, QString &errorMessage);

private:
    AppDbStorage *m_dbStorage;
    QList<QObject*> m_workouts;
};
