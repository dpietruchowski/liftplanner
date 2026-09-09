#pragma once

#include "exercise.h"
#include "exercisekind.h"
#include "restseconds.h"
#include "setprescription.h"
#include <QString>
#include <QStringList>
#include <vector>

class TemplateExercise final
{
public:
    TemplateExercise();
    explicit TemplateExercise(int definitionId);

    int definitionId() const;
    int position() const;
    int restSecondsOverride() const;
    const QString& notes() const;
    const std::vector<SetPrescription>& sets() const;
    std::vector<SetPrescription>& sets();

    void setDefinitionId(int definitionId);
    void setPosition(int position);
    void setRestSecondsOverride(int seconds);
    void setNotes(const QString& notes);

    void addSet(const SetPrescription& prescription, int atPosition = -1);
    void removeSet(int index);
    void moveSet(int from, int to);
    void normalizePositions();

    int effectiveRestSeconds(int definitionDefault) const;
    Exercise toExercise(const QString& name, ExerciseKind kind, int definitionRestSeconds) const;

    QStringList validationErrors() const;
    bool isValid() const;

private:
    int m_definitionId { -1 };
    int m_position { 0 };
    int m_restSecondsOverride { RestSeconds::inherited };
    QString m_notes;
    std::vector<SetPrescription> m_sets;
};
