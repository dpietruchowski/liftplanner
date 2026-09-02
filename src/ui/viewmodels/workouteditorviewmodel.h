#pragma once

#include "modules/workout/domain/entities/workout.h"
#include "ui/models/exercisedefinitionmodel.h"
#include "ui/models/workoutmodel.h"
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QStringList>

class WorkoutService;
class WorkoutTemplateService;

class WorkoutEditorViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(WorkoutModel* workout READ workout NOTIFY workoutChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY workoutChanged)
    Q_PROPERTY(QDateTime plannedTime READ plannedTime WRITE setPlannedTime NOTIFY workoutChanged)
    Q_PROPERTY(bool editing READ isEditing NOTIFY workoutChanged)
    Q_PROPERTY(int exerciseCount READ exerciseCount NOTIFY workoutChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY workoutChanged)
    Q_PROPERTY(QStringList validationErrors READ validationErrors NOTIFY workoutChanged)
    Q_PROPERTY(bool dirty READ isDirty NOTIFY dirtyChanged)

public:
    WorkoutEditorViewModel(WorkoutService* service, WorkoutTemplateService* templateService,
                           QObject* parent = nullptr);
    ~WorkoutEditorViewModel();

    WorkoutModel* workout() const;
    QString name() const;
    QDateTime plannedTime() const;
    bool isEditing() const;
    int exerciseCount() const;
    bool isValid() const;
    QStringList validationErrors() const;
    bool isDirty() const;

    void setName(const QString& value);
    void setPlannedTime(const QDateTime& value);

    Q_INVOKABLE void createNew(const QString& name, const QDateTime& plannedTime);
    Q_INVOKABLE void edit(int workoutId);
    Q_INVOKABLE void startFromTemplate(int templateId, const QDateTime& plannedTime);
    Q_INVOKABLE void discard();

    Q_INVOKABLE void addExercise(ExerciseDefinitionModel* definition);
    Q_INVOKABLE void removeExercise(int exerciseIndex);
    Q_INVOKABLE void moveExercise(int from, int to);
    Q_INVOKABLE void setExerciseRest(int exerciseIndex, int restSeconds);
    Q_INVOKABLE void setExerciseNotes(int exerciseIndex, const QString& notes);

    Q_INVOKABLE void addSet(int exerciseIndex);
    Q_INVOKABLE void duplicateSet(int exerciseIndex, int setIndex);
    Q_INVOKABLE void removeSet(int exerciseIndex, int setIndex);
    Q_INVOKABLE void moveSet(int exerciseIndex, int from, int to);
    Q_INVOKABLE void setSetRepetitions(int exerciseIndex, int setIndex, int repetitions);
    Q_INVOKABLE void setSetWeight(int exerciseIndex, int setIndex, double weight);
    Q_INVOKABLE void setSetDuration(int exerciseIndex, int setIndex, int seconds);
    Q_INVOKABLE void setSetDistance(int exerciseIndex, int setIndex, double meters);
    Q_INVOKABLE void adjustSetPrimary(int exerciseIndex, int setIndex, int steps);
    Q_INVOKABLE void adjustSetSecondary(int exerciseIndex, int setIndex, int steps);

    Q_INVOKABLE void save();
    Q_INVOKABLE void saveAsTemplate(const QString& name);

signals:
    void workoutChanged();
    void dirtyChanged();
    void saved(int workoutId);
    void savedAsTemplate(int templateId);
    void errorOccurred(const QString& errorMessage);

private:
    Exercise* exerciseAt(int exerciseIndex);
    Set* setAt(int exerciseIndex, int setIndex);
    Set seedSetFor(const ExerciseDefinitionModel& definition) const;
    Set seedSetFor(const Exercise& exercise) const;
    Set seedSet(SetMetric metric, LoadType loadType) const;

    void adopt(const Workout& workout, bool dirty);
    void publish();
    void markDirty();
    void setDirty(bool value);

    WorkoutService* m_service;
    WorkoutTemplateService* m_templateService;

    Workout m_workout;
    WorkoutModel* m_model { nullptr };
    bool m_editing { false };
    bool m_dirty { false };
};
