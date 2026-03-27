#pragma once

#include <QString>
#include <QDateTime>
#include <vector>
#include "exercise.h"

class Workout final
{
public:
    Workout();
    Workout(const QString &name, const QDateTime &createdTime);

    int id() const;
    const QString &name() const;
    const QDateTime &createdTime() const;
    const QDateTime &plannedTime() const;
    const QDateTime &startedTime() const;
    const QDateTime &endedTime() const;

    void setId(int id);
    void setName(const QString &name);
    void setCreatedTime(const QDateTime &createdTime);
    void setPlannedTime(const QDateTime &plannedTime);
    void setStartedTime(const QDateTime &startedTime);
    void setEndedTime(const QDateTime &endedTime);

    const std::vector<Exercise> &exercises() const;
    std::vector<Exercise> &exercises();
    void addExercise(const Exercise &exercise);
    void removeExercise(int index);
    bool isCompleted() const;

    static Workout createDefault(const QString &name);

private:
    void validate() const;

    int m_id{-1};
    QString m_name;
    QDateTime m_createdTime;
    QDateTime m_plannedTime;
    QDateTime m_startedTime;
    QDateTime m_endedTime;
    std::vector<Exercise> m_exercises;
};
