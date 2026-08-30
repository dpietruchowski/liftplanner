#include "exercisecatalogseed.h"
#include "modules/exercisecatalog/domain/repositories/exercisedefinitionquery.h"
#include "modules/exercisecatalog/domain/repositories/exercisedefinitionrepository.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSet>

namespace
{

constexpr const char* exercises_key = "exercises";
constexpr const char* slug_key = "slug";
constexpr const char* name_key = "name";
constexpr const char* aliases_key = "aliases";
constexpr const char* kind_key = "kind";
constexpr const char* equipment_key = "equipment";
constexpr const char* mechanics_key = "mechanics";
constexpr const char* laterality_key = "laterality";
constexpr const char* default_metric_key = "defaultMetric";
constexpr const char* default_load_type_key = "defaultLoadType";
constexpr const char* default_rest_seconds_key = "defaultRestSeconds";
constexpr const char* muscles_key = "muscles";
constexpr const char* muscle_key = "muscle";
constexpr const char* role_key = "role";
constexpr const char* instructions_key = "instructions";
constexpr const char* video_url_key = "videoUrl";

ExerciseDefinition definitionFromJson(const QJsonObject& object)
{
    ExerciseDefinition definition(object.value(slug_key).toString(),
                                  object.value(name_key).toString(),
                                  exerciseKindFromString(object.value(kind_key).toString()));

    definition.setEquipment(equipmentFromString(object.value(equipment_key).toString()));
    definition.setMechanics(mechanicsFromString(object.value(mechanics_key).toString()));
    definition.setLaterality(lateralityFromString(object.value(laterality_key).toString()));
    definition.setDefaultMetric(setMetricFromString(object.value(default_metric_key).toString()));
    definition.setDefaultLoadType(
        loadTypeFromString(object.value(default_load_type_key).toString()));
    definition.setInstructions(object.value(instructions_key).toString());
    definition.setVideoUrl(object.value(video_url_key).toString());
    definition.setOrigin(CatalogOrigin::BuiltIn);

    if (object.contains(default_rest_seconds_key))
        definition.setDefaultRestSeconds(object.value(default_rest_seconds_key).toInt());

    for (const QJsonValue& alias : object.value(aliases_key).toArray())
        definition.addAlias(alias.toString());

    for (const QJsonValue& entry : object.value(muscles_key).toArray())
    {
        const QJsonObject involvement = entry.toObject();
        definition.addMuscle({ muscleFromString(involvement.value(muscle_key).toString()),
                               muscleRoleFromString(involvement.value(role_key).toString()) });
    }

    return definition;
}

}

std::vector<ExerciseDefinition> ExerciseCatalogSeed::parse(const QByteArray& json,
                                                           QStringList& errors)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        errors.append(QStringLiteral("seed is not valid json: %1").arg(parseError.errorString()));
        return {};
    }

    const QJsonArray entries = document.object().value(exercises_key).toArray();
    if (entries.isEmpty())
    {
        errors.append(QStringLiteral("seed has no exercises"));
        return {};
    }

    std::vector<ExerciseDefinition> definitions;
    definitions.reserve(static_cast<size_t>(entries.size()));

    QSet<QString> seenSlugs;
    for (const QJsonValue& entry : entries)
    {
        const ExerciseDefinition definition = definitionFromJson(entry.toObject());

        const QStringList validationErrors = definition.validationErrors();
        if (!validationErrors.isEmpty())
        {
            errors.append(QStringLiteral("%1: %2").arg(
                definition.slug().isEmpty() ? definition.name() : definition.slug(),
                validationErrors.join(QStringLiteral("; "))));
            continue;
        }

        if (seenSlugs.contains(definition.slug()))
        {
            errors.append(QStringLiteral("%1: duplicated slug").arg(definition.slug()));
            continue;
        }

        seenSlugs.insert(definition.slug());
        definitions.push_back(definition);
    }

    return definitions;
}

CatalogSeedResult ExerciseCatalogSeed::sync(ExerciseDefinitionRepository& repository,
                                            const std::vector<ExerciseDefinition>& definitions)
{
    CatalogSeedResult result;

    QSet<QString> seededSlugs;
    for (const ExerciseDefinition& definition : definitions)
    {
        const bool known
            = repository.exists(ExerciseDefinitionQuery().whereSlug(definition.slug()));

        repository.save(definition);
        seededSlugs.insert(definition.slug());

        if (known)
            ++result.updated;
        else
            ++result.inserted;
    }

    const auto builtIns = repository.findAll(
        ExerciseDefinitionQuery().whereOrigin(CatalogOrigin::BuiltIn).whereArchived(false));

    for (ExerciseDefinition builtIn : builtIns)
    {
        if (seededSlugs.contains(builtIn.slug()))
            continue;

        builtIn.setArchived(true);
        repository.save(builtIn);
        ++result.archived;
    }

    return result;
}

CatalogSeedResult ExerciseCatalogSeed::apply(ExerciseDefinitionRepository& repository,
                                             const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        CatalogSeedResult result;
        result.errors.append(QStringLiteral("cannot read seed at %1").arg(path));
        return result;
    }

    QStringList errors;
    const std::vector<ExerciseDefinition> definitions = parse(file.readAll(), errors);

    CatalogSeedResult result = sync(repository, definitions);
    result.errors = errors;
    return result;
}
