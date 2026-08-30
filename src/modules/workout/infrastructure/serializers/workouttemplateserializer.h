#pragma once

#include <QVariantMap>

class WorkoutTemplate;
class TemplateExercise;
class SetPrescription;

class WorkoutTemplateSerializer
{
public:
    static constexpr const char* table = "workout_templates";
    static constexpr const char* id_key = "id";
    static constexpr const char* name_key = "name";
    static constexpr const char* notes_key = "notes";

    static WorkoutTemplate fromVariant(const QVariantMap& data);
    static QVariantMap toVariant(const WorkoutTemplate& workoutTemplate);
};

class TemplateExerciseSerializer
{
public:
    static constexpr const char* table = "template_exercises";
    static constexpr const char* id_key = "id";
    static constexpr const char* template_id_key = "template_id";
    static constexpr const char* definition_id_key = "definition_id";
    static constexpr const char* position_key = "position";
    static constexpr const char* rest_seconds_override_key = "rest_seconds_override";
    static constexpr const char* notes_key = "notes";

    static TemplateExercise fromVariant(const QVariantMap& data);
    static QVariantMap toVariant(const TemplateExercise& exercise, int templateId);
};

class SetPrescriptionSerializer
{
public:
    static constexpr const char* table = "set_prescriptions";
    static constexpr const char* id_key = "id";
    static constexpr const char* template_exercise_id_key = "template_exercise_id";
    static constexpr const char* metric_key = "metric";
    static constexpr const char* load_type_key = "load_type";
    static constexpr const char* repetitions_key = "repetitions";
    static constexpr const char* weight_key = "weight";
    static constexpr const char* duration_seconds_key = "duration_seconds";
    static constexpr const char* distance_meters_key = "distance_meters";
    static constexpr const char* rest_seconds_override_key = "rest_seconds_override";
    static constexpr const char* position_key = "position";

    static SetPrescription fromVariant(const QVariantMap& data);
    static QVariantMap toVariant(const SetPrescription& prescription, int templateExerciseId);
};
