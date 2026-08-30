#include "workouttemplateserializer.h"
#include "modules/workout/domain/entities/setprescription.h"
#include "modules/workout/domain/entities/templateexercise.h"
#include "modules/workout/domain/entities/workouttemplate.h"

WorkoutTemplate WorkoutTemplateSerializer::fromVariant(const QVariantMap& data)
{
    WorkoutTemplate workoutTemplate;

    if (data.contains(id_key))
        workoutTemplate.setId(data.value(id_key).toInt());
    if (data.contains(name_key))
        workoutTemplate.setName(data.value(name_key).toString());
    if (data.contains(notes_key))
        workoutTemplate.setNotes(data.value(notes_key).toString());

    return workoutTemplate;
}

QVariantMap WorkoutTemplateSerializer::toVariant(const WorkoutTemplate& workoutTemplate)
{
    QVariantMap data;

    if (workoutTemplate.id() != -1)
        data.insert(id_key, workoutTemplate.id());

    data.insert(name_key, workoutTemplate.name());
    data.insert(notes_key, workoutTemplate.notes());

    return data;
}

TemplateExercise TemplateExerciseSerializer::fromVariant(const QVariantMap& data)
{
    TemplateExercise exercise;

    if (data.contains(definition_id_key))
        exercise.setDefinitionId(data.value(definition_id_key).toInt());
    if (data.contains(position_key))
        exercise.setPosition(data.value(position_key).toInt());
    if (data.contains(rest_seconds_override_key))
        exercise.setRestSecondsOverride(data.value(rest_seconds_override_key).toInt());
    if (data.contains(notes_key))
        exercise.setNotes(data.value(notes_key).toString());

    return exercise;
}

QVariantMap TemplateExerciseSerializer::toVariant(const TemplateExercise& exercise, int templateId)
{
    QVariantMap data;

    data.insert(template_id_key, templateId);
    data.insert(definition_id_key, exercise.definitionId());
    data.insert(position_key, exercise.position());
    data.insert(rest_seconds_override_key, exercise.restSecondsOverride());
    data.insert(notes_key, exercise.notes());

    return data;
}

SetPrescription SetPrescriptionSerializer::fromVariant(const QVariantMap& data)
{
    SetPrescription prescription;

    if (data.contains(metric_key))
        prescription.setMetric(setMetricFromString(data.value(metric_key).toString()));
    if (data.contains(load_type_key))
        prescription.setLoadType(loadTypeFromString(data.value(load_type_key).toString()));
    if (data.contains(repetitions_key))
        prescription.setRepetitions(data.value(repetitions_key).toInt());
    if (data.contains(weight_key))
        prescription.setWeight(data.value(weight_key).toDouble());
    if (data.contains(duration_seconds_key))
        prescription.setDurationSeconds(data.value(duration_seconds_key).toInt());
    if (data.contains(distance_meters_key))
        prescription.setDistanceMeters(data.value(distance_meters_key).toDouble());
    if (data.contains(rest_seconds_override_key))
        prescription.setRestSecondsOverride(data.value(rest_seconds_override_key).toInt());
    if (data.contains(position_key))
        prescription.setPosition(data.value(position_key).toInt());

    return prescription;
}

QVariantMap SetPrescriptionSerializer::toVariant(const SetPrescription& prescription,
                                                 int templateExerciseId)
{
    QVariantMap data;

    data.insert(template_exercise_id_key, templateExerciseId);
    data.insert(metric_key, setMetricToString(prescription.metric()));
    data.insert(load_type_key, loadTypeToString(prescription.loadType()));
    data.insert(repetitions_key, prescription.repetitions());
    data.insert(weight_key, prescription.weight());
    data.insert(duration_seconds_key, prescription.durationSeconds());
    data.insert(distance_meters_key, prescription.distanceMeters());
    data.insert(rest_seconds_override_key, prescription.restSecondsOverride());
    data.insert(position_key, prescription.position());

    return data;
}
