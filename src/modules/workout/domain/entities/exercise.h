#pragma once

#include "exercisekind.h"
#include "set.h"
#include <QString>
#include <vector>

class Exercise final
{
public:
    Exercise();
    Exercise(const QString& name, int restSeconds);

    int id() const;
    int workoutId() const;
    const QString& name() const;
    const QString& description() const;
    int restSeconds() const;
    ExerciseKind kind() const;
    int restSecondsForSet(int index) const;

    void setId(int id);
    void setWorkoutId(int workoutId);
    void setName(const QString& name);
    void setDescription(const QString& description);
    void setRestSeconds(int restSeconds);
    void setKind(ExerciseKind kind);

    const std::vector<Set>& sets() const;
    std::vector<Set>& sets();
    void addSet(const Set& set);
    void removeSet(int index);
    bool isCompleted() const;
    bool isWeighted() const;
    QString setsToString() const;

    double totalWeight() const;
    int totalRepetitions() const;
    int totalDurationSeconds() const;
    double totalDistanceMeters() const;
    double averageWeight() const;
    double bestOneRepMax() const;

private:
    void validate() const;

    int m_id { -1 };
    int m_workoutId { -1 };
    QString m_name;
    QString m_description;
    int m_restSeconds { 120 };
    ExerciseKind m_kind { ExerciseKind::Strength };
    std::vector<Set> m_sets;
};
