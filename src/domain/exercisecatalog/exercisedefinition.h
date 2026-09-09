#pragma once

#include "catalogorigin.h"
#include "domain/workout/exercisekind.h"
#include "domain/workout/loadtype.h"
#include "domain/workout/setmetric.h"
#include "equipment.h"
#include "laterality.h"
#include "mechanics.h"
#include "muscleinvolvement.h"
#include <QString>
#include <QStringList>
#include <vector>

class ExerciseDefinition final
{
public:
    ExerciseDefinition();
    ExerciseDefinition(const QString& slug, const QString& name, ExerciseKind kind);

    static ExerciseDefinition createImported(const QString& name, ExerciseKind kind);
    static QString slugify(const QString& text);

    int id() const;
    const QString& slug() const;
    const QString& name() const;
    const QStringList& aliases() const;
    ExerciseKind kind() const;
    LoadType defaultLoadType() const;
    SetMetric defaultMetric() const;
    const std::vector<MuscleInvolvement>& muscles() const;
    Equipment equipment() const;
    Mechanics mechanics() const;
    Laterality laterality() const;
    int defaultRestSeconds() const;
    const QString& videoUrl() const;
    const QString& instructions() const;
    CatalogOrigin origin() const;
    bool isArchived() const;

    void setId(int id);
    void setSlug(const QString& slug);
    void setName(const QString& name);
    void setKind(ExerciseKind kind);
    void setDefaultLoadType(LoadType loadType);
    void setDefaultMetric(SetMetric metric);
    void setEquipment(Equipment equipment);
    void setMechanics(Mechanics mechanics);
    void setLaterality(Laterality laterality);
    void setDefaultRestSeconds(int seconds);
    void setVideoUrl(const QString& videoUrl);
    void setInstructions(const QString& instructions);
    void setOrigin(CatalogOrigin origin);
    void setArchived(bool archived);

    void addAlias(const QString& alias);
    void addMuscle(const MuscleInvolvement& involvement);
    void absorb(const ExerciseDefinition& other);

    bool matchesName(const QString& text) const;
    bool targets(Muscle muscle) const;
    std::vector<Muscle> musclesWithRole(MuscleRole role) const;
    std::vector<BodyRegion> regions() const;
    bool isDeletable() const;

    QStringList validationErrors() const;
    bool isValid() const;

private:
    static bool isValidSlug(const QString& slug);

    int m_id { -1 };
    QString m_slug;
    QString m_name;
    QStringList m_aliases;
    ExerciseKind m_kind { ExerciseKind::Strength };
    LoadType m_defaultLoadType { LoadType::External };
    SetMetric m_defaultMetric { SetMetric::Reps };
    std::vector<MuscleInvolvement> m_muscles;
    Equipment m_equipment { Equipment::Other };
    Mechanics m_mechanics { Mechanics::Compound };
    Laterality m_laterality { Laterality::Bilateral };
    int m_defaultRestSeconds { 120 };
    QString m_videoUrl;
    QString m_instructions;
    CatalogOrigin m_origin { CatalogOrigin::BuiltIn };
    bool m_archived { false };
};
