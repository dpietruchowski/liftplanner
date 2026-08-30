#include "exercisedefinitionserializer.h"
#include "modules/exercisecatalog/domain/entities/exercisedefinition.h"

ExerciseDefinition ExerciseDefinitionSerializer::fromVariant(const QVariantMap& data)
{
    ExerciseDefinition definition;

    if (data.contains(id_key))
        definition.setId(data.value(id_key).toInt());
    if (data.contains(slug_key))
        definition.setSlug(data.value(slug_key).toString());
    if (data.contains(name_key))
        definition.setName(data.value(name_key).toString());
    if (data.contains(kind_key))
        definition.setKind(exerciseKindFromString(data.value(kind_key).toString()));
    if (data.contains(default_load_type_key))
        definition.setDefaultLoadType(
            loadTypeFromString(data.value(default_load_type_key).toString()));
    if (data.contains(default_metric_key))
        definition.setDefaultMetric(setMetricFromString(data.value(default_metric_key).toString()));
    if (data.contains(equipment_key))
        definition.setEquipment(equipmentFromString(data.value(equipment_key).toString()));
    if (data.contains(mechanics_key))
        definition.setMechanics(mechanicsFromString(data.value(mechanics_key).toString()));
    if (data.contains(laterality_key))
        definition.setLaterality(lateralityFromString(data.value(laterality_key).toString()));
    if (data.contains(default_rest_seconds_key))
        definition.setDefaultRestSeconds(data.value(default_rest_seconds_key).toInt());
    if (data.contains(video_url_key))
        definition.setVideoUrl(data.value(video_url_key).toString());
    if (data.contains(instructions_key))
        definition.setInstructions(data.value(instructions_key).toString());
    if (data.contains(origin_key))
        definition.setOrigin(catalogOriginFromString(data.value(origin_key).toString()));
    if (data.contains(archived_key))
        definition.setArchived(data.value(archived_key).toBool());

    const QString aliases = data.value(aliases_key).toString();
    for (const QString& alias : aliases.split(QLatin1String(alias_separator), Qt::SkipEmptyParts))
        definition.addAlias(alias);

    return definition;
}

QVariantMap ExerciseDefinitionSerializer::toVariant(const ExerciseDefinition& definition)
{
    QVariantMap data;

    if (definition.id() != -1)
        data.insert(id_key, definition.id());

    data.insert(slug_key, definition.slug());
    data.insert(name_key, definition.name());
    data.insert(aliases_key, definition.aliases().join(QLatin1String(alias_separator)));
    data.insert(kind_key, exerciseKindToString(definition.kind()));
    data.insert(default_load_type_key, loadTypeToString(definition.defaultLoadType()));
    data.insert(default_metric_key, setMetricToString(definition.defaultMetric()));
    data.insert(equipment_key, equipmentToString(definition.equipment()));
    data.insert(mechanics_key, mechanicsToString(definition.mechanics()));
    data.insert(laterality_key, lateralityToString(definition.laterality()));
    data.insert(default_rest_seconds_key, definition.defaultRestSeconds());
    data.insert(video_url_key, definition.videoUrl());
    data.insert(instructions_key, definition.instructions());
    data.insert(origin_key, catalogOriginToString(definition.origin()));
    data.insert(archived_key, definition.isArchived() ? 1 : 0);

    return data;
}
