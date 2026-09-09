#pragma once

#include <QDateTime>
#include <QString>
#include <optional>
#include <vector>

#include "async/service.h"
#include "async/testing.h"
#include "domain/workout/workout.h"
#include "domain/workout/workouttemplate.h"
#include "domain/workout/workouttemplaterow.h"

class ExerciseDefinitionLookup;
class WorkoutTemplateRepository;
class WorkoutTemplateRowRepository;

class WorkoutTemplateService final : public Service
{
public:
    WorkoutTemplateService(WorkoutTemplateRepository& repository,
                           WorkoutTemplateRowRepository& rowRepository,
                           const ExerciseDefinitionLookup& lookup, QObject* worker);

    Task<std::vector<WorkoutTemplateRow>> searchRows(const QString& text);
    Task<std::optional<WorkoutTemplate>> findById(int id);

    Task<int> save(const WorkoutTemplate& workoutTemplate);
    Task<int> saveFromWorkout(const Workout& workout, const QString& name);
    Task<int> duplicate(int id, const QString& name);
    Task<bool> remove(int id);

    Task<Workout> instantiate(int id, const QDateTime& plannedTime);

private:
    LIBS_TEST_FRIEND(WorkoutTemplateServiceTest)

    Result<std::vector<WorkoutTemplateRow>> searchRowsCore(const QString& text);
    Result<std::optional<WorkoutTemplate>> findByIdCore(int id);
    Result<int> saveCore(WorkoutTemplate workoutTemplate);
    Result<int> saveFromWorkoutCore(const Workout& workout, const QString& name);
    Result<int> duplicateCore(int id, const QString& name);
    Result<bool> removeCore(int id);
    Result<Workout> instantiateCore(int id, const QDateTime& plannedTime);

    WorkoutTemplateRepository& m_repository;
    WorkoutTemplateRowRepository& m_rowRepository;
    const ExerciseDefinitionLookup& m_lookup;
};
