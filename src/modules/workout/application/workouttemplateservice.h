#pragma once

#include <QDateTime>
#include <QString>
#include <optional>
#include <vector>

#include "modules/workout/domain/entities/workout.h"
#include "modules/workout/domain/entities/workouttemplate.h"
#include "utils/service.h"
#include "utils/testing.h"

class ExerciseDefinitionLookup;
class WorkoutTemplateRepository;

struct WorkoutTemplateSummary final
{
    WorkoutTemplate workoutTemplate;
    QStringList exerciseNames;
    int setCount { 0 };
    bool complete { true };
};

class WorkoutTemplateService final : public Service
{
public:
    WorkoutTemplateService(WorkoutTemplateRepository& repository,
                           const ExerciseDefinitionLookup& lookup, QObject* worker);

    Task<std::vector<WorkoutTemplate>> loadTemplates();
    Task<std::vector<WorkoutTemplate>> searchTemplates(const QString& text);
    Task<std::vector<WorkoutTemplateSummary>> loadSummaries();
    Task<std::vector<WorkoutTemplateSummary>> searchSummaries(const QString& text);
    Task<std::optional<WorkoutTemplate>> findById(int id);

    Task<int> save(const WorkoutTemplate& workoutTemplate);
    Task<int> saveFromWorkout(const Workout& workout, const QString& name);
    Task<int> duplicate(int id, const QString& name);
    Task<bool> remove(int id);

    Task<Workout> instantiate(int id, const QDateTime& plannedTime);

private:
    LIBS_TEST_FRIEND(WorkoutTemplateServiceTest)

    Result<std::vector<WorkoutTemplate>> loadTemplatesCore();
    Result<std::vector<WorkoutTemplate>> searchTemplatesCore(const QString& text);
    Result<std::vector<WorkoutTemplateSummary>> loadSummariesCore();
    Result<std::vector<WorkoutTemplateSummary>> searchSummariesCore(const QString& text);
    std::vector<WorkoutTemplateSummary> summarize(const std::vector<WorkoutTemplate>& templates);
    Result<std::optional<WorkoutTemplate>> findByIdCore(int id);
    Result<int> saveCore(WorkoutTemplate workoutTemplate);
    Result<int> saveFromWorkoutCore(const Workout& workout, const QString& name);
    Result<int> duplicateCore(int id, const QString& name);
    Result<bool> removeCore(int id);
    Result<Workout> instantiateCore(int id, const QDateTime& plannedTime);

    WorkoutTemplateRepository& m_repository;
    const ExerciseDefinitionLookup& m_lookup;
};
