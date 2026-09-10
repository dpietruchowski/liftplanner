#pragma once

#include "domain/workout/exercise.h"
#include "setmodel.h"
#include <QDateTime>
#include <QList>
#include <QObject>
#include <QQmlListProperty>

class ExerciseModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY dataChanged)
    Q_PROPERTY(QString description READ description NOTIFY dataChanged)
    Q_PROPERTY(int restSeconds READ restSeconds NOTIFY dataChanged)
    Q_PROPERTY(QString restText READ restText NOTIFY dataChanged)
    Q_PROPERTY(QString kind READ kindString NOTIFY dataChanged)
    Q_PROPERTY(QQmlListProperty<SetModel> sets READ setsProperty NOTIFY setsChanged)
    Q_PROPERTY(bool completed READ isCompleted NOTIFY completedChanged)
    Q_PROPERTY(QString previousSummary READ previousSummary NOTIFY previousPerformanceChanged)
    Q_PROPERTY(QDateTime previousDate READ previousDate NOTIFY previousPerformanceChanged)

public:
    explicit ExerciseModel(QObject* parent = nullptr);
    explicit ExerciseModel(const Exercise& exercise, QObject* parent = nullptr);

    QString name() const;
    QString description() const;
    int restSeconds() const;
    QString restText() const;
    QString kindString() const;
    bool isCompleted() const;
    QString previousSummary() const;
    QDateTime previousDate() const;
    void setPreviousPerformance(const QString& summary, const QDateTime& date);

    QQmlListProperty<SetModel> setsProperty();
    QList<SetModel*> sets() const;

    Q_INVOKABLE QString setsToString() const;

    void addSet(SetModel* set);
    void removeSet(SetModel* set);

    Exercise toEntity() const;

signals:
    void dataChanged();
    void setsChanged();
    void completedChanged();
    void previousPerformanceChanged();

private:
    Exercise m_record;
    QList<SetModel*> m_sets;
    QString m_previousSummary;
    QDateTime m_previousDate;
};
