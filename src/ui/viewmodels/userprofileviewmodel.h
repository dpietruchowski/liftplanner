#pragma once

#include "modules/userprofile/domain/entities/userprofile.h"
#include <QObject>
#include <QString>

class UserProfileService;

class UserProfileViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString sex READ sex WRITE setSex NOTIFY profileChanged)
    Q_PROPERTY(int sessionsPerWeek READ sessionsPerWeek WRITE setSessionsPerWeek NOTIFY
                   profileChanged)
    Q_PROPERTY(QString experienceLevel READ experienceLevel WRITE setExperienceLevel NOTIFY
                   profileChanged)
    Q_PROPERTY(QString primaryGoal READ primaryGoal WRITE setPrimaryGoal NOTIFY profileChanged)
    Q_PROPERTY(QString dateOfBirth READ dateOfBirth WRITE setDateOfBirth NOTIFY profileChanged)
    Q_PROPERTY(int age READ age NOTIFY profileChanged)
    Q_PROPERTY(double bodyweight READ bodyweight WRITE setBodyweight NOTIFY profileChanged)
    Q_PROPERTY(QString bodyweightUnit READ bodyweightUnit NOTIFY profileChanged)
    Q_PROPERTY(QString unitSystem READ unitSystem WRITE setUnitSystem NOTIFY profileChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY profileChanged)
    Q_PROPERTY(QString timezone READ timezone WRITE setTimezone NOTIFY profileChanged)
    Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY profileChanged)
    Q_PROPERTY(bool dirty READ isDirty NOTIFY dirtyChanged)

public:
    explicit UserProfileViewModel(UserProfileService* service, QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void save();

    QString sex() const;
    int sessionsPerWeek() const;
    QString experienceLevel() const;
    QString primaryGoal() const;
    QString dateOfBirth() const;
    int age() const;
    double bodyweight() const;
    QString bodyweightUnit() const;
    QString unitSystem() const;
    QString language() const;
    QString timezone() const;
    QString notes() const;
    bool isDirty() const;

    void setSex(const QString& sex);
    void setSessionsPerWeek(int sessionsPerWeek);
    void setExperienceLevel(const QString& experienceLevel);
    void setPrimaryGoal(const QString& primaryGoal);
    void setDateOfBirth(const QString& isoDate);
    void setBodyweight(double bodyweight);
    void setUnitSystem(const QString& unitSystem);
    void setLanguage(const QString& language);
    void setTimezone(const QString& timezone);
    void setNotes(const QString& notes);

signals:
    void profileChanged();
    void dirtyChanged();
    void saved();

private:
    void markDirty();

    UserProfileService* m_service;
    UserProfile m_profile;
    bool m_dirty { false };
};
