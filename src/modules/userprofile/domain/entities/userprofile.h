#pragma once

#include <optional>
#include <QString>
#include "sex.h"
#include "experiencelevel.h"
#include "primarygoal.h"
#include "unitsystem.h"

class UserProfile final
{
public:
    UserProfile();

    int userId() const;
    const QString &language() const;
    const QString &timezone() const;
    Sex sex() const;
    int sessionsPerWeek() const;
    ExperienceLevel experienceLevel() const;
    PrimaryGoal primaryGoal() const;
    std::optional<double> bodyweightKg() const;
    UnitSystem unitSystem() const;
    const QString &notes() const;

    void setUserId(int userId);
    void setLanguage(const QString &language);
    void setTimezone(const QString &timezone);
    void setSex(Sex sex);
    void setSessionsPerWeek(int sessionsPerWeek);
    void setExperienceLevel(ExperienceLevel experienceLevel);
    void setPrimaryGoal(PrimaryGoal primaryGoal);
    void setBodyweightKg(std::optional<double> bodyweightKg);
    void setUnitSystem(UnitSystem unitSystem);
    void setNotes(const QString &notes);

    static UserProfile createDefault();

private:
    int m_userId{-1};
    QString m_language{QStringLiteral("en")};
    QString m_timezone{QStringLiteral("UTC")};
    Sex m_sex{Sex::Other};
    int m_sessionsPerWeek{3};
    ExperienceLevel m_experienceLevel{ExperienceLevel::Beginner};
    PrimaryGoal m_primaryGoal{PrimaryGoal::GeneralFitness};
    std::optional<double> m_bodyweightKg;
    UnitSystem m_unitSystem{UnitSystem::Metric};
    QString m_notes;
};
