#include "exercisecatalogservice.h"
#include "domain/exercisecatalog/exercisedefinitionrepository.h"
#include "domain/exercisecatalog/exercisematcher.h"

ExerciseCatalogService::ExerciseCatalogService(ExerciseDefinitionRepository& repository,
                                               QObject* worker)
    : Service(worker)
    , m_repository(repository)
{
}

Task<std::vector<ExerciseDefinition>>
ExerciseCatalogService::search(const ExerciseDefinitionQuery& query)
{
    return invoke([this, query] { return searchCore(query); });
}

Task<std::optional<ExerciseDefinition>> ExerciseCatalogService::findById(int id)
{
    return invoke([this, id] { return findByIdCore(id); });
}

Task<std::optional<ExerciseDefinition>> ExerciseCatalogService::findBySlug(const QString& slug)
{
    return invoke([this, slug] { return findBySlugCore(slug); });
}

Task<int> ExerciseCatalogService::saveCustom(const ExerciseDefinition& definition)
{
    return invoke([this, definition] { return saveCustomCore(definition); });
}

Task<std::optional<int>> ExerciseCatalogService::match(const QString& name)
{
    return invoke([this, name] { return matchCore(name); });
}

Task<int> ExerciseCatalogService::matchOrImport(const QString& name, ExerciseKind kind)
{
    return invoke([this, name, kind] { return matchOrImportCore(name, kind); });
}

Task<bool> ExerciseCatalogService::archive(int id)
{
    return invoke([this, id] { return archiveCore(id); });
}

Task<bool> ExerciseCatalogService::restore(int id)
{
    return invoke([this, id] { return restoreCore(id); });
}

Task<bool> ExerciseCatalogService::remove(int id)
{
    return invoke([this, id] { return removeCore(id); });
}

Result<std::vector<ExerciseDefinition>>
ExerciseCatalogService::searchCore(ExerciseDefinitionQuery query)
{
    if (!query.archived().has_value())
        query.whereArchived(false);
    if (!query.orderByNameDirection().has_value())
        query.orderByName(SortDirection::Ascending);

    return Result<std::vector<ExerciseDefinition>>::success(m_repository.findAll(query));
}

Result<std::optional<ExerciseDefinition>> ExerciseCatalogService::findByIdCore(int id)
{
    return Result<std::optional<ExerciseDefinition>>::success(
        m_repository.findOne(ExerciseDefinitionQuery().whereId(id)));
}

Result<std::optional<ExerciseDefinition>>
ExerciseCatalogService::findBySlugCore(const QString& slug)
{
    return Result<std::optional<ExerciseDefinition>>::success(
        m_repository.findOne(ExerciseDefinitionQuery().whereSlug(slug)));
}

Result<int> ExerciseCatalogService::saveCustomCore(ExerciseDefinition definition)
{
    if (definition.slug().isEmpty())
        definition.setSlug(ExerciseDefinition::slugify(definition.name()));

    if (definition.origin() == CatalogOrigin::BuiltIn)
        definition.setOrigin(CatalogOrigin::Custom);

    const QStringList errors = definition.validationErrors();
    if (!errors.isEmpty())
        return Result<int>::failure(errors.join(QStringLiteral("; ")));

    const auto existing
        = m_repository.findOne(ExerciseDefinitionQuery().whereSlug(definition.slug()));

    if (existing.has_value() && existing->origin() == CatalogOrigin::BuiltIn)
    {
        return Result<int>::failure(
            QStringLiteral("%1 is a built-in exercise and cannot be overwritten")
                .arg(definition.slug()));
    }

    if (existing.has_value() && definition.id() == -1)
        definition.setId(existing->id());

    return Result<int>::success(m_repository.save(definition));
}

Result<std::optional<int>> ExerciseCatalogService::matchCore(const QString& name)
{
    const auto candidates = ExerciseMatcher::toCandidates(activeDefinitions());
    return Result<std::optional<int>>::success(ExerciseMatcher::match(name, candidates));
}

Result<int> ExerciseCatalogService::matchOrImportCore(const QString& name, ExerciseKind kind)
{
    if (name.trimmed().isEmpty())
        return Result<int>::failure(QStringLiteral("an exercise needs a name"));

    const auto matched = matchCore(name);
    if (matched.value().has_value())
        return Result<int>::success(matched.value().value());

    ExerciseDefinition imported = ExerciseDefinition::createImported(name, kind);

    const auto collision
        = m_repository.findOne(ExerciseDefinitionQuery().whereSlug(imported.slug()));
    if (collision.has_value())
        return Result<int>::success(collision->id());

    return Result<int>::success(m_repository.save(imported));
}

Result<bool> ExerciseCatalogService::archiveCore(int id) { return setArchived(id, true); }

Result<bool> ExerciseCatalogService::restoreCore(int id) { return setArchived(id, false); }

Result<bool> ExerciseCatalogService::removeCore(int id)
{
    const auto definition = m_repository.findOne(ExerciseDefinitionQuery().whereId(id));
    if (!definition.has_value())
        return Result<bool>::success(false);

    if (!definition->isDeletable())
    {
        return Result<bool>::failure(
            QStringLiteral("%1 is a built-in exercise and can only be archived")
                .arg(definition->slug()));
    }

    return Result<bool>::success(m_repository.remove(ExerciseDefinitionQuery().whereId(id)));
}

Result<bool> ExerciseCatalogService::setArchived(int id, bool archived)
{
    auto definition = m_repository.findOne(ExerciseDefinitionQuery().whereId(id));
    if (!definition.has_value())
        return Result<bool>::success(false);

    definition->setArchived(archived);
    m_repository.save(definition.value());
    return Result<bool>::success(true);
}

std::vector<ExerciseDefinition> ExerciseCatalogService::activeDefinitions() const
{
    return m_repository.findAll(ExerciseDefinitionQuery().whereArchived(false));
}
