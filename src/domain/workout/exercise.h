#pragma once

#include "exercisekind.h"
#include "set.h"
#include <QString>
#include <QStringList>
#include <optional>
#include <vector>

class Exercise final
{
public:
    Exercise();
    Exercise(const QString& name, int restSeconds);

    static Exercise createFromDefinition(int definitionId, const QString& name, ExerciseKind kind,
                                         int restSeconds);
    static Exercise createAdHoc(const QString& name, ExerciseKind kind, int restSeconds);

    int id() const;
    int workoutId() const;
    const QString& name() const;
    const QString& description() const;
    int restSeconds() const;
    ExerciseKind kind() const;
    int position() const;
    const std::optional<int>& definitionId() const;
    bool hasDefinition() const;
    const QString& notes() const;
    int restSecondsForSet(int index) const;

    void setId(int id);
    void setWorkoutId(int workoutId);
    void setName(const QString& name);
    void setDescription(const QString& description);
    void setRestSeconds(int restSeconds);
    void setKind(ExerciseKind kind);
    void setPosition(int position);
    void setDefinitionId(int definitionId);
    void clearDefinitionId();
    void setNotes(const QString& notes);

    const std::vector<Set>& sets() const;
    std::vector<Set>& sets();
    void addSet(const Set& set, int atPosition = -1);
    void removeSet(int index);
    void moveSet(int from, int to);
    void duplicateSet(int index);
    void normalizePositions();
    bool isCompleted() const;
    bool isWeighted() const;
    bool acceptsSet(const Set& set) const;
    QStringList validationErrors() const;
    bool isValid() const;
    QString setsToString() const;

    double totalWeight() const;
    int totalRepetitions() const;
    int totalDurationSeconds() const;
    double totalDistanceMeters() const;
    double averageWeight() const;
    double bestOneRepMax() const;

private:
    void validate() const;
    void renumberSets();

    int m_id { -1 };
    int m_workoutId { -1 };
    QString m_name;
    QString m_description;
    int m_restSeconds { 120 };
    int m_position { 0 };
    std::optional<int> m_definitionId;
    QString m_notes;
    ExerciseKind m_kind { ExerciseKind::Strength };
    std::vector<Set> m_sets;
};
