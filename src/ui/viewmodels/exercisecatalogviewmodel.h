#pragma once

#include "application/workout/workoutservice.h"
#include "ui/models/exercisedefinitionmodel.h"
#include <QList>
#include <QObject>
#include <QString>
#include <vector>

class ExerciseCatalogService;

class ExerciseCatalogViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<ExerciseDefinitionModel*> exercises READ exercises NOTIFY exercisesChanged)
    Q_PROPERTY(int count READ count NOTIFY exercisesChanged)
    Q_PROPERTY(int recentCount READ recentCount NOTIFY exercisesChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY filtersChanged)
    Q_PROPERTY(QString muscle READ muscle WRITE setMuscle NOTIFY filtersChanged)
    Q_PROPERTY(QString region READ region WRITE setRegion NOTIFY filtersChanged)
    Q_PROPERTY(QString equipment READ equipment WRITE setEquipment NOTIFY filtersChanged)
    Q_PROPERTY(QString kind READ kind WRITE setKind NOTIFY filtersChanged)
    Q_PROPERTY(bool filtered READ isFiltered NOTIFY filtersChanged)

public:
    explicit ExerciseCatalogViewModel(ExerciseCatalogService* service,
                                      WorkoutService* workoutService = nullptr,
                                      QObject* parent = nullptr);
    ~ExerciseCatalogViewModel();

    QList<ExerciseDefinitionModel*> exercises() const;
    int count() const;
    int recentCount() const;
    bool isLoading() const;

    QString searchText() const;
    QString muscle() const;
    QString region() const;
    QString equipment() const;
    QString kind() const;
    bool isFiltered() const;

    void setSearchText(const QString& value);
    void setMuscle(const QString& value);
    void setRegion(const QString& value);
    void setEquipment(const QString& value);
    void setKind(const QString& value);

    Q_INVOKABLE void load();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void clearFilters();
    Q_INVOKABLE ExerciseDefinitionModel* findById(int definitionId) const;

signals:
    void exercisesChanged();
    void loadingChanged();
    void filtersChanged();
    void errorOccurred(const QString& errorMessage);

private:
    void applyFilterChange(QString& target, const QString& value);
    void setLoading(bool value);
    void showExercises(const std::vector<ExerciseDefinition>& definitions);

    ExerciseCatalogService* m_service;
    WorkoutService* m_workoutService;
    QList<ExerciseDefinitionModel*> m_exercises;
    std::vector<WorkoutService::RecentExercise> m_recent;
    int m_recentCount { 0 };

    QString m_searchText;
    QString m_muscle;
    QString m_region;
    QString m_equipment;
    QString m_kind;
    bool m_loading { false };
};
