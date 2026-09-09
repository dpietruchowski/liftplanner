#pragma once

#include "domain/exercisecatalog/exercisedefinition.h"
#include <QObject>
#include <QString>
#include <QStringList>

class ExerciseDefinitionModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int definitionId READ definitionId NOTIFY dataChanged)
    Q_PROPERTY(QString slug READ slug NOTIFY dataChanged)
    Q_PROPERTY(QString name READ name NOTIFY dataChanged)
    Q_PROPERTY(QString kind READ kind NOTIFY dataChanged)
    Q_PROPERTY(QString equipment READ equipment NOTIFY dataChanged)
    Q_PROPERTY(QString defaultMetric READ defaultMetric NOTIFY dataChanged)
    Q_PROPERTY(QString defaultLoadType READ defaultLoadType NOTIFY dataChanged)
    Q_PROPERTY(int defaultRestSeconds READ defaultRestSeconds NOTIFY dataChanged)
    Q_PROPERTY(QStringList primaryMuscles READ primaryMuscles NOTIFY dataChanged)
    Q_PROPERTY(QStringList secondaryMuscles READ secondaryMuscles NOTIFY dataChanged)
    Q_PROPERTY(QStringList regions READ regions NOTIFY dataChanged)
    Q_PROPERTY(QString videoUrl READ videoUrl NOTIFY dataChanged)
    Q_PROPERTY(QString instructions READ instructions NOTIFY dataChanged)
    Q_PROPERTY(bool custom READ isCustom NOTIFY dataChanged)
    Q_PROPERTY(bool archived READ isArchived NOTIFY dataChanged)

public:
    explicit ExerciseDefinitionModel(QObject* parent = nullptr);
    explicit ExerciseDefinitionModel(const ExerciseDefinition& definition,
                                     QObject* parent = nullptr);

    int definitionId() const;
    QString slug() const;
    QString name() const;
    QString kind() const;
    QString equipment() const;
    QString defaultMetric() const;
    QString defaultLoadType() const;
    int defaultRestSeconds() const;
    QStringList primaryMuscles() const;
    QStringList secondaryMuscles() const;
    QStringList regions() const;
    QString videoUrl() const;
    QString instructions() const;
    bool isCustom() const;
    bool isArchived() const;

    const ExerciseDefinition& entity() const;

signals:
    void dataChanged();

private:
    QStringList musclesWithRole(MuscleRole role) const;

    ExerciseDefinition m_definition;
};
