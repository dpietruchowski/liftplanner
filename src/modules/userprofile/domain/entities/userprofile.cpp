#include "userprofile.h"

UserProfile::UserProfile() = default;

int UserProfile::userId() const { return m_userId; }
const QString &UserProfile::language() const { return m_language; }
const QString &UserProfile::timezone() const { return m_timezone; }
Sex UserProfile::sex() const { return m_sex; }
int UserProfile::sessionsPerWeek() const { return m_sessionsPerWeek; }
ExperienceLevel UserProfile::experienceLevel() const { return m_experienceLevel; }
PrimaryGoal UserProfile::primaryGoal() const { return m_primaryGoal; }
std::optional<QDate> UserProfile::dateOfBirth() const { return m_dateOfBirth; }
std::optional<double> UserProfile::bodyweightKg() const { return m_bodyweightKg; }
UnitSystem UserProfile::unitSystem() const { return m_unitSystem; }
const QString &UserProfile::notes() const { return m_notes; }

void UserProfile::setUserId(int userId) { m_userId = userId; }
void UserProfile::setLanguage(const QString &language) { m_language = language; }
void UserProfile::setTimezone(const QString &timezone) { m_timezone = timezone; }
void UserProfile::setSex(Sex sex) { m_sex = sex; }
void UserProfile::setSessionsPerWeek(int sessionsPerWeek) { m_sessionsPerWeek = sessionsPerWeek; }
void UserProfile::setExperienceLevel(ExperienceLevel experienceLevel) { m_experienceLevel = experienceLevel; }
void UserProfile::setPrimaryGoal(PrimaryGoal primaryGoal) { m_primaryGoal = primaryGoal; }
void UserProfile::setDateOfBirth(std::optional<QDate> dateOfBirth) { m_dateOfBirth = dateOfBirth; }
void UserProfile::setBodyweightKg(std::optional<double> bodyweightKg) { m_bodyweightKg = bodyweightKg; }
void UserProfile::setUnitSystem(UnitSystem unitSystem) { m_unitSystem = unitSystem; }
void UserProfile::setNotes(const QString &notes) { m_notes = notes; }

UserProfile UserProfile::createDefault()
{
    return UserProfile{};
}
