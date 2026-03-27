#include "exercise.h"

Exercise::Exercise() = default;

Exercise::Exercise(const QString &name, int restSeconds)
    : m_name(name), m_restSeconds(restSeconds)
{
    validate();
}

int Exercise::id() const { return m_id; }
int Exercise::workoutId() const { return m_workoutId; }
const QString &Exercise::name() const { return m_name; }
const QString &Exercise::description() const { return m_description; }
int Exercise::restSeconds() const { return m_restSeconds; }
const QString &Exercise::youtubeLink() const { return m_youtubeLink; }

void Exercise::setId(int id) { m_id = id; }
void Exercise::setWorkoutId(int workoutId) { m_workoutId = workoutId; }
void Exercise::setName(const QString &name) { m_name = name; }
void Exercise::setDescription(const QString &description) { m_description = description; }
void Exercise::setRestSeconds(int restSeconds) { m_restSeconds = restSeconds; }
void Exercise::setYoutubeLink(const QString &youtubeLink) { m_youtubeLink = youtubeLink; }

void Exercise::validate() const
{
}
