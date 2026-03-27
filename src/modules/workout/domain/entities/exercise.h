#pragma once

#include <QString>
#include <vector>
#include "set.h"

class Exercise final
{
public:
    Exercise();
    Exercise(const QString &name, int restSeconds);

    int id() const;
    int workoutId() const;
    const QString &name() const;
    const QString &description() const;
    int restSeconds() const;
    const QString &youtubeLink() const;

    void setId(int id);
    void setWorkoutId(int workoutId);
    void setName(const QString &name);
    void setDescription(const QString &description);
    void setRestSeconds(int restSeconds);
    void setYoutubeLink(const QString &youtubeLink);

    const std::vector<Set> &sets() const;
    std::vector<Set> &sets();
    void addSet(const Set &set);
    void removeSet(int index);
    bool isCompleted() const;
    QString setsToString() const;

private:
    void validate() const;

    int m_id{-1};
    int m_workoutId{-1};
    QString m_name;
    QString m_description;
    int m_restSeconds{120};
    QString m_youtubeLink;
    std::vector<Set> m_sets;
};
