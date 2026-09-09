#pragma once

#include "templateexercise.h"
#include <QString>
#include <QStringList>
#include <vector>

class WorkoutTemplate final
{
public:
    WorkoutTemplate();
    explicit WorkoutTemplate(const QString& name);

    int id() const;
    const QString& name() const;
    const QString& notes() const;
    const std::vector<TemplateExercise>& exercises() const;
    std::vector<TemplateExercise>& exercises();

    void setId(int id);
    void setName(const QString& name);
    void setNotes(const QString& notes);

    void addExercise(const TemplateExercise& exercise, int atPosition = -1);
    void removeExercise(int index);
    void moveExercise(int from, int to);
    void normalizePositions();

    bool isEmpty() const;
    bool references(int definitionId) const;
    std::vector<int> definitionIds() const;

    QStringList validationErrors() const;
    bool isValid() const;

private:
    int m_id { -1 };
    QString m_name;
    QString m_notes;
    std::vector<TemplateExercise> m_exercises;
};
