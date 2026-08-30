#pragma once

#include "modules/exercisecatalog/domain/entities/exercisedefinition.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <vector>

class ExerciseDefinitionRepository;

struct CatalogSeedResult
{
    int inserted { 0 };
    int updated { 0 };
    int archived { 0 };
    QStringList errors;

    bool isOk() const { return errors.isEmpty(); }
};

class ExerciseCatalogSeed
{
public:
    static constexpr const char* resource_path = ":/LiftPlanner/data/exercises.json";

    static std::vector<ExerciseDefinition> parse(const QByteArray& json, QStringList& errors);
    static CatalogSeedResult sync(ExerciseDefinitionRepository& repository,
                                  const std::vector<ExerciseDefinition>& definitions);
    static CatalogSeedResult apply(ExerciseDefinitionRepository& repository,
                                   const QString& path
                                   = QString::fromLatin1(ExerciseCatalogSeed::resource_path));
};
