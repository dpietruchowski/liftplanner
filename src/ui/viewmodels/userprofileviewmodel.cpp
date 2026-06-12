#include "userprofileviewmodel.h"
#include "modules/userprofile/application/userprofileservice.h"
#include <QDate>
#include <QLocale>
#include <QTimeZone>
#include <cmath>

namespace
{
constexpr double kg_per_lb = 0.45359237;
constexpr int default_user_id = 1;

double roundToTenth(double value)
{
    return std::round(value * 10.0) / 10.0;
}
}

UserProfileViewModel::UserProfileViewModel(UserProfileService* service, QObject* parent)
    : QObject(parent)
    , m_service(service)
{
    load();
}

void UserProfileViewModel::load()
{
    auto loaded = m_service ? m_service->load() : std::nullopt;
    if (loaded.has_value())
    {
        m_profile = *loaded;
    }
    else
    {
        m_profile = UserProfile::createDefault();
        m_profile.setUserId(default_user_id);
        m_profile.setTimezone(QString::fromUtf8(QTimeZone::systemTimeZoneId()));
        m_profile.setLanguage(QLocale::system().name().left(2));
    }

    m_dirty = false;
    emit profileChanged();
    emit dirtyChanged();
}

void UserProfileViewModel::save()
{
    if (!m_service)
        return;

    if (m_profile.userId() < 0)
        m_profile.setUserId(default_user_id);

    m_service->save(m_profile);
    m_dirty = false;
    emit dirtyChanged();
    emit saved();
}

QString UserProfileViewModel::sex() const { return sexToString(m_profile.sex()); }

int UserProfileViewModel::sessionsPerWeek() const { return m_profile.sessionsPerWeek(); }

QString UserProfileViewModel::experienceLevel() const
{
    return experienceLevelToString(m_profile.experienceLevel());
}

QString UserProfileViewModel::primaryGoal() const
{
    return primaryGoalToString(m_profile.primaryGoal());
}

QString UserProfileViewModel::dateOfBirth() const
{
    return m_profile.dateOfBirth().has_value() ? m_profile.dateOfBirth()->toString(Qt::ISODate)
                                               : QString();
}

int UserProfileViewModel::age() const
{
    if (!m_profile.dateOfBirth().has_value())
        return -1;

    const QDate today = QDate::currentDate();
    const QDate dob = *m_profile.dateOfBirth();
    int years = today.year() - dob.year();
    if (dob.addYears(years) > today)
        --years;
    return years;
}

double UserProfileViewModel::bodyweight() const
{
    if (!m_profile.bodyweightKg().has_value())
        return 0.0;

    const double kg = *m_profile.bodyweightKg();
    return roundToTenth(m_profile.unitSystem() == UnitSystem::Imperial ? kg / kg_per_lb : kg);
}

QString UserProfileViewModel::bodyweightUnit() const
{
    return m_profile.unitSystem() == UnitSystem::Imperial ? QStringLiteral("lb")
                                                          : QStringLiteral("kg");
}

QString UserProfileViewModel::unitSystem() const
{
    return unitSystemToString(m_profile.unitSystem());
}

QString UserProfileViewModel::language() const { return m_profile.language(); }

QString UserProfileViewModel::timezone() const { return m_profile.timezone(); }

QString UserProfileViewModel::notes() const { return m_profile.notes(); }

bool UserProfileViewModel::isDirty() const { return m_dirty; }

void UserProfileViewModel::setSex(const QString& sex)
{
    if (sexFromString(sex) == m_profile.sex())
        return;
    m_profile.setSex(sexFromString(sex));
    markDirty();
}

void UserProfileViewModel::setSessionsPerWeek(int sessionsPerWeek)
{
    if (sessionsPerWeek == m_profile.sessionsPerWeek())
        return;
    m_profile.setSessionsPerWeek(sessionsPerWeek);
    markDirty();
}

void UserProfileViewModel::setExperienceLevel(const QString& experienceLevel)
{
    if (experienceLevelFromString(experienceLevel) == m_profile.experienceLevel())
        return;
    m_profile.setExperienceLevel(experienceLevelFromString(experienceLevel));
    markDirty();
}

void UserProfileViewModel::setPrimaryGoal(const QString& primaryGoal)
{
    if (primaryGoalFromString(primaryGoal) == m_profile.primaryGoal())
        return;
    m_profile.setPrimaryGoal(primaryGoalFromString(primaryGoal));
    markDirty();
}

void UserProfileViewModel::setDateOfBirth(const QString& isoDate)
{
    std::optional<QDate> date;
    if (!isoDate.trimmed().isEmpty())
    {
        const QDate parsed = QDate::fromString(isoDate.trimmed(), Qt::ISODate);
        if (!parsed.isValid())
            return;
        date = parsed;
    }

    if (date == m_profile.dateOfBirth())
        return;
    m_profile.setDateOfBirth(date);
    markDirty();
}

void UserProfileViewModel::setBodyweight(double bodyweight)
{
    std::optional<double> kg;
    if (bodyweight > 0.0)
        kg = m_profile.unitSystem() == UnitSystem::Imperial ? bodyweight * kg_per_lb : bodyweight;

    const auto current = m_profile.bodyweightKg();
    if (kg.has_value() == current.has_value()
        && (!kg.has_value() || std::abs(*kg - *current) < 0.01))
        return;

    m_profile.setBodyweightKg(kg);
    markDirty();
}

void UserProfileViewModel::setUnitSystem(const QString& unitSystem)
{
    if (unitSystemFromString(unitSystem) == m_profile.unitSystem())
        return;
    m_profile.setUnitSystem(unitSystemFromString(unitSystem));
    markDirty();
}

void UserProfileViewModel::setLanguage(const QString& language)
{
    if (language == m_profile.language())
        return;
    m_profile.setLanguage(language);
    markDirty();
}

void UserProfileViewModel::setTimezone(const QString& timezone)
{
    if (timezone == m_profile.timezone())
        return;
    m_profile.setTimezone(timezone);
    markDirty();
}

void UserProfileViewModel::setNotes(const QString& notes)
{
    if (notes == m_profile.notes())
        return;
    m_profile.setNotes(notes);
    markDirty();
}

void UserProfileViewModel::markDirty()
{
    emit profileChanged();
    if (m_dirty)
        return;
    m_dirty = true;
    emit dirtyChanged();
}
