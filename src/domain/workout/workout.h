#pragma once

#include "exercise.h"
#include "workoutstatus.h"
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <vector>

class Workout final
{
public:
    Workout();
    Workout(const QString& name, const QDateTime& createdTime);

    int id() const;
    const QString& name() const;
    const QDateTime& createdTime() const;
    const QDateTime& plannedTime() const;
    const QDateTime& startedTime() const;
    const QDateTime& endedTime() const;
    WorkoutStatus status() const;
    bool hasGeneratedName() const;

    void setId(int id);
    void setName(const QString& name);
    void setGeneratedName(bool generated);
    void setCreatedTime(const QDateTime& createdTime);
    void setPlannedTime(const QDateTime& plannedTime);
    void setStartedTime(const QDateTime& startedTime);
    void setEndedTime(const QDateTime& endedTime);
    void setStatus(WorkoutStatus status);

    void start();
    void end();
    void returnToPlan();

    const std::vector<Exercise>& exercises() const;
    std::vector<Exercise>& exercises();
    void addExercise(const Exercise& exercise, int atPosition = -1);
    void removeExercise(int index);
    void moveExercise(int from, int to);
    void normalizePositions();
    bool isCompleted() const;
    bool isEmpty() const;
    QStringList validationErrors() const;
    bool isValid() const;

    int totalRepetitions() const;
    int totalSets() const;
    double totalWeight() const;
    int totalDurationSeconds() const;
    double totalDistanceMeters() const;

    static Workout createDefault(const QString& name);

private:
    int m_id { -1 };
    QString m_name;
    QDateTime m_createdTime;
    QDateTime m_plannedTime;
    QDateTime m_startedTime;
    QDateTime m_endedTime;
    WorkoutStatus m_status { WorkoutStatus::Planned };
    bool m_generatedName { false };
    std::vector<Exercise> m_exercises;
};
