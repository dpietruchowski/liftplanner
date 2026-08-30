#pragma once

#include "core/types.h"
#include "modules/exercisecatalog/domain/entities/catalogorigin.h"
#include "modules/exercisecatalog/domain/entities/equipment.h"
#include "modules/exercisecatalog/domain/entities/muscle.h"
#include "modules/workout/domain/entities/exercisekind.h"
#include <QString>
#include <optional>

class ExerciseDefinitionQuery final
{
public:
    ExerciseDefinitionQuery& whereId(int id);
    ExerciseDefinitionQuery& whereSlug(const QString& slug);
    ExerciseDefinitionQuery& whereNameContains(const QString& text);
    ExerciseDefinitionQuery& whereKind(ExerciseKind kind);
    ExerciseDefinitionQuery& whereEquipment(Equipment equipment);
    ExerciseDefinitionQuery& whereMuscle(Muscle muscle);
    ExerciseDefinitionQuery& whereRegion(BodyRegion region);
    ExerciseDefinitionQuery& whereOrigin(CatalogOrigin origin);
    ExerciseDefinitionQuery& whereArchived(bool archived);
    ExerciseDefinitionQuery& orderByName(SortDirection direction);
    ExerciseDefinitionQuery& withLimit(int limit);
    ExerciseDefinitionQuery& withOffset(int offset);

    const std::optional<int>& id() const;
    const std::optional<QString>& slug() const;
    const std::optional<QString>& nameContains() const;
    const std::optional<ExerciseKind>& kind() const;
    const std::optional<Equipment>& equipment() const;
    const std::optional<Muscle>& muscle() const;
    const std::optional<BodyRegion>& region() const;
    const std::optional<CatalogOrigin>& origin() const;
    const std::optional<bool>& archived() const;
    const std::optional<SortDirection>& orderByNameDirection() const;
    const std::optional<int>& limit() const;
    const std::optional<int>& offset() const;

private:
    std::optional<int> m_id;
    std::optional<QString> m_slug;
    std::optional<QString> m_nameContains;
    std::optional<ExerciseKind> m_kind;
    std::optional<Equipment> m_equipment;
    std::optional<Muscle> m_muscle;
    std::optional<BodyRegion> m_region;
    std::optional<CatalogOrigin> m_origin;
    std::optional<bool> m_archived;
    std::optional<SortDirection> m_orderByNameDirection;
    std::optional<int> m_limit;
    std::optional<int> m_offset;
};
