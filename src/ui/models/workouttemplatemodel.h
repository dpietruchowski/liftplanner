#pragma once

#include "domain/workout/workouttemplaterow.h"
#include <QObject>
#include <QString>
#include <QStringList>

class WorkoutTemplateModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int templateId READ templateId NOTIFY dataChanged)
    Q_PROPERTY(QString name READ name NOTIFY dataChanged)
    Q_PROPERTY(QString notes READ notes NOTIFY dataChanged)
    Q_PROPERTY(QStringList exerciseNames READ exerciseNames NOTIFY dataChanged)
    Q_PROPERTY(int exerciseCount READ exerciseCount NOTIFY dataChanged)
    Q_PROPERTY(int setCount READ setCount NOTIFY dataChanged)
    Q_PROPERTY(bool complete READ isComplete NOTIFY dataChanged)

public:
    explicit WorkoutTemplateModel(QObject* parent = nullptr);
    explicit WorkoutTemplateModel(const WorkoutTemplateRow& row, QObject* parent = nullptr);

    int templateId() const;
    QString name() const;
    QString notes() const;
    QStringList exerciseNames() const;
    int exerciseCount() const;
    int setCount() const;
    bool isComplete() const;

    const WorkoutTemplateRow& entity() const;

signals:
    void dataChanged();

private:
    WorkoutTemplateRow m_row;
};
