#include "exercisedefinition.h"
#include "modules/workout/domain/entities/setcompatibility.h"
#include "utils/textmatcher.h"
#include <algorithm>

ExerciseDefinition::ExerciseDefinition() = default;

ExerciseDefinition::ExerciseDefinition(const QString& slug, const QString& name, ExerciseKind kind)
    : m_slug(slug)
    , m_name(name)
    , m_kind(kind)
    , m_defaultLoadType(defaultLoadTypeFor(kind))
    , m_defaultMetric(defaultMetricFor(kind))
{
}

ExerciseDefinition ExerciseDefinition::createImported(const QString& name, ExerciseKind kind)
{
    ExerciseDefinition definition(slugify(name), name, kind);
    definition.m_origin = CatalogOrigin::Imported;
    return definition;
}

QString ExerciseDefinition::slugify(const QString& text)
{
    const QString folded = TextMatcher::foldAccents(text).toLower();

    QString result;
    result.reserve(folded.size());
    for (QChar character : folded)
    {
        if (character.unicode() < 128 && character.isLetterOrNumber())
            result.append(character);
        else if (!result.isEmpty() && !result.endsWith(QLatin1Char('-')))
            result.append(QLatin1Char('-'));
    }

    while (result.endsWith(QLatin1Char('-')))
        result.chop(1);

    return result;
}

int ExerciseDefinition::id() const { return m_id; }
const QString& ExerciseDefinition::slug() const { return m_slug; }
const QString& ExerciseDefinition::name() const { return m_name; }
const QStringList& ExerciseDefinition::aliases() const { return m_aliases; }
ExerciseKind ExerciseDefinition::kind() const { return m_kind; }
LoadType ExerciseDefinition::defaultLoadType() const { return m_defaultLoadType; }
SetMetric ExerciseDefinition::defaultMetric() const { return m_defaultMetric; }
const std::vector<MuscleInvolvement>& ExerciseDefinition::muscles() const { return m_muscles; }
Equipment ExerciseDefinition::equipment() const { return m_equipment; }
Mechanics ExerciseDefinition::mechanics() const { return m_mechanics; }
Laterality ExerciseDefinition::laterality() const { return m_laterality; }
int ExerciseDefinition::defaultRestSeconds() const { return m_defaultRestSeconds; }
const QString& ExerciseDefinition::videoUrl() const { return m_videoUrl; }
const QString& ExerciseDefinition::instructions() const { return m_instructions; }
CatalogOrigin ExerciseDefinition::origin() const { return m_origin; }
bool ExerciseDefinition::isArchived() const { return m_archived; }

void ExerciseDefinition::setId(int id) { m_id = id; }
void ExerciseDefinition::setSlug(const QString& slug) { m_slug = slug; }
void ExerciseDefinition::setName(const QString& name) { m_name = name; }
void ExerciseDefinition::setKind(ExerciseKind kind) { m_kind = kind; }
void ExerciseDefinition::setDefaultLoadType(LoadType loadType) { m_defaultLoadType = loadType; }
void ExerciseDefinition::setDefaultMetric(SetMetric metric) { m_defaultMetric = metric; }
void ExerciseDefinition::setEquipment(Equipment equipment) { m_equipment = equipment; }
void ExerciseDefinition::setMechanics(Mechanics mechanics) { m_mechanics = mechanics; }
void ExerciseDefinition::setLaterality(Laterality laterality) { m_laterality = laterality; }
void ExerciseDefinition::setDefaultRestSeconds(int seconds) { m_defaultRestSeconds = seconds; }
void ExerciseDefinition::setVideoUrl(const QString& videoUrl) { m_videoUrl = videoUrl; }
void ExerciseDefinition::setInstructions(const QString& instructions)
{
    m_instructions = instructions;
}
void ExerciseDefinition::setOrigin(CatalogOrigin origin) { m_origin = origin; }
void ExerciseDefinition::setArchived(bool archived) { m_archived = archived; }

void ExerciseDefinition::addAlias(const QString& alias)
{
    const QString trimmed = alias.trimmed();
    if (trimmed.isEmpty())
        return;

    if (TextMatcher::compare(trimmed, m_name, true))
        return;

    for (const QString& existing : m_aliases)
    {
        if (TextMatcher::compare(trimmed, existing, true))
            return;
    }

    m_aliases.append(trimmed);
}

void ExerciseDefinition::addMuscle(const MuscleInvolvement& involvement)
{
    for (auto& existing : m_muscles)
    {
        if (existing.muscle == involvement.muscle)
        {
            existing.role = involvement.role;
            return;
        }
    }

    m_muscles.push_back(involvement);
}

void ExerciseDefinition::absorb(const ExerciseDefinition& other)
{
    addAlias(other.m_name);
    for (const QString& alias : other.m_aliases)
        addAlias(alias);
}

bool ExerciseDefinition::matchesName(const QString& text) const
{
    if (TextMatcher::compare(text, m_name, true))
        return true;

    for (const QString& alias : m_aliases)
    {
        if (TextMatcher::compare(text, alias, true))
            return true;
    }

    return false;
}

bool ExerciseDefinition::targets(Muscle muscle) const
{
    return std::any_of(m_muscles.cbegin(), m_muscles.cend(),
                       [muscle](const MuscleInvolvement& involvement)
                       { return involvement.muscle == muscle; });
}

std::vector<Muscle> ExerciseDefinition::musclesWithRole(MuscleRole role) const
{
    std::vector<Muscle> result;
    for (const auto& involvement : m_muscles)
    {
        if (involvement.role == role)
            result.push_back(involvement.muscle);
    }
    return result;
}

std::vector<BodyRegion> ExerciseDefinition::regions() const
{
    std::vector<BodyRegion> result;
    for (const auto& involvement : m_muscles)
    {
        const BodyRegion region = regionOf(involvement.muscle);
        if (std::find(result.cbegin(), result.cend(), region) == result.cend())
            result.push_back(region);
    }
    return result;
}

bool ExerciseDefinition::isDeletable() const { return m_origin != CatalogOrigin::BuiltIn; }

bool ExerciseDefinition::isValidSlug(const QString& slug)
{
    if (slug.isEmpty())
        return false;
    if (slug.startsWith(QLatin1Char('-')) || slug.endsWith(QLatin1Char('-')))
        return false;
    if (slug.contains(QStringLiteral("--")))
        return false;

    for (QChar character : slug)
    {
        const bool lowerLetter = character >= QLatin1Char('a') && character <= QLatin1Char('z');
        const bool digit = character >= QLatin1Char('0') && character <= QLatin1Char('9');
        if (!lowerLetter && !digit && character != QLatin1Char('-'))
            return false;
    }

    return true;
}

QStringList ExerciseDefinition::validationErrors() const
{
    QStringList errors;

    if (!isValidSlug(m_slug))
        errors.append(QStringLiteral("slug must be non-empty lower-case kebab-case"));

    if (m_name.trimmed().isEmpty())
        errors.append(QStringLiteral("name must not be empty"));

    if (m_defaultRestSeconds < 0)
        errors.append(QStringLiteral("default rest seconds must not be negative"));

    if (!metricSuitsKind(m_kind, m_defaultMetric))
    {
        errors.append(QStringLiteral("default metric %1 does not suit kind %2")
                          .arg(setMetricToString(m_defaultMetric), exerciseKindToString(m_kind)));
    }

    const bool needsPrimaryMuscle
        = m_kind == ExerciseKind::Strength || m_kind == ExerciseKind::Bodyweight;
    if (needsPrimaryMuscle && musclesWithRole(MuscleRole::Primary).empty())
        errors.append(QStringLiteral("strength movements need at least one primary muscle"));

    return errors;
}

bool ExerciseDefinition::isValid() const { return validationErrors().isEmpty(); }
