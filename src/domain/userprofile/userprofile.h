#pragma once

#include "experiencelevel.h"
#include "primarygoal.h"
#include "sex.h"
#include "unitsystem.h"
#include <QDate>
#include <QString>
#include <optional>

class UserProfile final
{
public:
    UserProfile();

    int userId() const;
    const QString& language() const;
    const QString& timezone() const;
    Sex sex() const;
    int sessionsPerWeek() const;
    ExperienceLevel experienceLevel() const;
    PrimaryGoal primaryGoal() const;
    std::optional<QDate> dateOfBirth() const;
    std::optional<double> bodyweightKg() const;
    UnitSystem unitSystem() const;
    const QString& notes() const;

    void setUserId(int userId);
    void setLanguage(const QString& language);
    void setTimezone(const QString& timezone);
    void setSex(Sex sex);
    void setSessionsPerWeek(int sessionsPerWeek);
    void setExperienceLevel(ExperienceLevel experienceLevel);
    void setPrimaryGoal(PrimaryGoal primaryGoal);
    void setDateOfBirth(std::optional<QDate> dateOfBirth);
    void setBodyweightKg(std::optional<double> bodyweightKg);
    void setUnitSystem(UnitSystem unitSystem);
    void setNotes(const QString& notes);

    static UserProfile createDefault();

private:
    int m_userId { -1 };
    QString m_language { QStringLiteral("en") };
    QString m_timezone { QStringLiteral("UTC") };
    Sex m_sex { Sex::Other };
    int m_sessionsPerWeek { 3 };
    ExperienceLevel m_experienceLevel { ExperienceLevel::Beginner };
    PrimaryGoal m_primaryGoal { PrimaryGoal::GeneralFitness };
    std::optional<QDate> m_dateOfBirth;
    std::optional<double> m_bodyweightKg;
    UnitSystem m_unitSystem { UnitSystem::Metric };
    QString m_notes;
};
