#pragma once

#include <QString>
#include <QStringList>

struct WorkoutTemplateRow final
{
    int id { -1 };
    QString name;
    QString notes;
    QStringList exerciseNames;
    int exerciseCount { 0 };
    int setCount { 0 };
    bool complete { true };
};
