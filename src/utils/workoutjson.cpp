#include "workoutjson.h"
#include "modules/workout/domain/entities/workoutstatus.h"
#include <QJsonDocument>
#include <QRegularExpression>
#include <cmath>

namespace WorkoutJson
{

QJsonObject setToJson(const Set& set)
{
    QJsonObject obj;
    if (set.id() != -1)
        obj["id"] = set.id();
    if (set.exerciseId() != -1)
        obj["exercise_id"] = set.exerciseId();
    obj["repetitions"] = set.repetitions();
    obj["weight"] = set.weight();
    obj["completed"] = set.completed();
    obj["metric"] = setMetricToString(set.metric());
    obj["load_type"] = loadTypeToString(set.loadType());
    obj["duration_seconds"] = set.durationSeconds();
    obj["distance_meters"] = set.distanceMeters();
    obj["rest_seconds_override"] = set.restSecondsOverride();
    return obj;
}

QJsonObject exerciseToJson(const Exercise& exercise)
{
    QJsonObject obj;
    if (exercise.id() != -1)
        obj["id"] = exercise.id();
    if (exercise.workoutId() != -1)
        obj["workout_id"] = exercise.workoutId();
    obj["name"] = exercise.name();
    obj["description"] = exercise.description();
    obj["rest_seconds"] = exercise.restSeconds();
    obj["kind"] = exerciseKindToString(exercise.kind());
    if (exercise.hasDefinition())
        obj["definition_id"] = exercise.definitionId().value();
    if (!exercise.notes().isEmpty())
        obj["notes"] = exercise.notes();

    QJsonArray setsArray;
    for (const auto& set : exercise.sets())
        setsArray.append(setToJson(set));
    obj["sets"] = setsArray;

    return obj;
}

QJsonObject workoutToJson(const Workout& workout)
{
    QJsonObject obj;
    if (workout.id() != -1)
        obj["id"] = workout.id();
    obj["name"] = workout.name();

    if (workout.createdTime().isValid())
        obj["created_time"] = workout.createdTime().toString(Qt::ISODate);
    if (workout.plannedTime().isValid())
        obj["planned_time"] = workout.plannedTime().toString(Qt::ISODate);
    if (workout.startedTime().isValid())
        obj["started_time"] = workout.startedTime().toString(Qt::ISODate);
    if (workout.endedTime().isValid())
        obj["ended_time"] = workout.endedTime().toString(Qt::ISODate);

    obj["status"] = workoutStatusToString(workout.status());

    QJsonArray exercisesArray;
    for (const auto& exercise : workout.exercises())
        exercisesArray.append(exerciseToJson(exercise));
    obj["exercises"] = exercisesArray;

    return obj;
}

QJsonObject exerciseToJsonCompact(const Exercise& exercise)
{
    QJsonObject obj;
    obj["name"] = exercise.name();
    obj["rest_seconds"] = exercise.restSeconds();
    obj["kind"] = exerciseKindToString(exercise.kind());
    obj["sets"] = exercise.setsToString();
    return obj;
}

QJsonObject workoutToJsonCompact(const Workout& workout)
{
    QJsonObject obj;
    obj["name"] = workout.name();

    if (workout.plannedTime().isValid())
        obj["planned_time"] = workout.plannedTime().toString(Qt::ISODate);
    if (workout.startedTime().isValid())
        obj["started_time"] = workout.startedTime().toString(Qt::ISODate);
    if (workout.endedTime().isValid())
        obj["ended_time"] = workout.endedTime().toString(Qt::ISODate);

    QJsonArray exercisesArray;
    for (const auto& exercise : workout.exercises())
        exercisesArray.append(exerciseToJsonCompact(exercise));
    obj["exercises"] = exercisesArray;

    return obj;
}

Set setFromJson(const QJsonObject& json)
{
    Set s;
    if (json.contains("id"))
        s.setId(json["id"].toInt());
    if (json.contains("exercise_id"))
        s.setExerciseId(json["exercise_id"].toInt());
    if (json.contains("repetitions"))
        s.setRepetitions(json["repetitions"].toInt());
    if (json.contains("weight"))
        s.setWeight(json["weight"].toDouble());
    if (json.contains("completed"))
        s.setCompleted(json["completed"].toBool());
    if (json.contains("metric"))
        s.setMetric(setMetricFromString(json["metric"].toString()));
    if (json.contains("load_type"))
        s.setLoadType(loadTypeFromString(json["load_type"].toString()));
    if (json.contains("duration_seconds"))
        s.setDurationSeconds(json["duration_seconds"].toInt());
    if (json.contains("distance_meters"))
        s.setDistanceMeters(json["distance_meters"].toDouble());
    if (json.contains("rest_seconds_override"))
        s.setRestSecondsOverride(json["rest_seconds_override"].toInt());
    return s;
}

static bool parseTimeToken(const QString& token, int& seconds)
{
    static const QRegularExpression re(QStringLiteral("^([0-9]+(?:\\.[0-9]+)?)(s|sec|min)$"));
    const auto match = re.match(token);
    if (!match.hasMatch())
        return false;

    const double value = match.captured(1).toDouble();
    const double scale = match.captured(2) == QStringLiteral("min") ? 60.0 : 1.0;
    seconds = static_cast<int>(std::llround(value * scale));
    return true;
}

static bool parseDistanceToken(const QString& token, double& meters)
{
    static const QRegularExpression re(QStringLiteral("^([0-9]+(?:\\.[0-9]+)?)(m|km)$"));
    const auto match = re.match(token);
    if (!match.hasMatch())
        return false;

    const double value = match.captured(1).toDouble();
    meters = match.captured(2) == QStringLiteral("km") ? value * 1000.0 : value;
    return true;
}

static bool parseRepsToken(const QString& token, Set& set)
{
    static const QRegularExpression re(
        QStringLiteral("^([0-9]+)x(?:bw(\\+|-)([0-9]+(?:\\.[0-9]+)?)kg"
                       "|(bw)|(band)|([0-9]+(?:\\.[0-9]+)?)kg)$"));
    const auto match = re.match(token);
    if (!match.hasMatch())
        return false;

    set.setRepetitions(match.captured(1).toInt());
    set.setMetric(SetMetric::Reps);

    if (!match.captured(2).isEmpty())
    {
        set.setLoadType(match.captured(2) == QStringLiteral("+") ? LoadType::Added
                                                                 : LoadType::Assisted);
        set.setWeight(match.captured(3).toDouble());
    }
    else if (!match.captured(4).isEmpty())
    {
        set.setLoadType(LoadType::Bodyweight);
    }
    else if (!match.captured(5).isEmpty())
    {
        set.setLoadType(LoadType::Band);
    }
    else
    {
        set.setLoadType(LoadType::External);
        set.setWeight(match.captured(6).toDouble());
    }
    return true;
}

static bool parseWorkToken(const QString& token, Set& set)
{
    if (parseRepsToken(token, set))
        return true;

    const int at = token.indexOf(QLatin1Char('@'));
    if (at > 0)
    {
        double meters = 0.0;
        int seconds = 0;
        if (!parseDistanceToken(token.left(at), meters)
            || !parseTimeToken(token.mid(at + 1), seconds))
            return false;

        set = Set::createDistance(meters, seconds);
        return true;
    }

    int seconds = 0;
    if (parseTimeToken(token, seconds))
    {
        set = Set::createDuration(seconds);
        return true;
    }

    double meters = 0.0;
    if (parseDistanceToken(token, meters))
    {
        set = Set::createDistance(meters, 0);
        return true;
    }

    return false;
}

std::vector<Set> parseSets(const QString& text, QStringList* errors)
{
    static const QRegularExpression repeatRe(QStringLiteral("^([0-9]+)x\\((.+)\\)$"));
    static const QRegularExpression whitespaceRe(QStringLiteral("\\s"));

    constexpr int max_repeat_count = 100;

    std::vector<Set> sets;

    const QStringList rawTokens = text.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const QString& rawToken : rawTokens)
    {
        const QString token = QString(rawToken).remove(whitespaceRe).toLower();
        if (token.isEmpty())
            continue;

        const auto reportUnparsed = [&errors, &rawToken]()
        {
            if (errors)
                errors->append(QStringLiteral("Unrecognized set: '%1'").arg(rawToken.trimmed()));
        };

        int count = 1;
        int restOverride = -1;
        QString work = token;

        const auto repeat = repeatRe.match(token);
        if (repeat.hasMatch())
        {
            count = repeat.captured(1).toInt();
            if (count < 1 || count > max_repeat_count)
            {
                reportUnparsed();
                continue;
            }
            work = repeat.captured(2);
        }

        const int separator = work.lastIndexOf(QLatin1Char('/'));
        if (separator >= 0)
        {
            if (!parseTimeToken(work.mid(separator + 1), restOverride))
            {
                reportUnparsed();
                continue;
            }
            work = work.left(separator);
        }

        Set set;
        if (!parseWorkToken(work, set))
        {
            reportUnparsed();
            continue;
        }

        set.setRestSecondsOverride(restOverride);
        for (int i = 0; i < count; ++i)
            sets.push_back(set);
    }

    return sets;
}

Exercise exerciseFromJson(const QJsonObject& json, QStringList* errors)
{
    Exercise e;
    if (json.contains("id"))
        e.setId(json["id"].toInt());
    if (json.contains("workout_id"))
        e.setWorkoutId(json["workout_id"].toInt());
    if (json.contains("name"))
        e.setName(json["name"].toString());
    if (json.contains("description"))
        e.setDescription(json["description"].toString());
    if (json.contains("rest_seconds"))
        e.setRestSeconds(json["rest_seconds"].toInt());
    if (json.contains("kind"))
        e.setKind(exerciseKindFromString(json["kind"].toString()));
    if (json.contains("definition_id"))
        e.setDefinitionId(json["definition_id"].toInt());
    if (json.contains("notes"))
        e.setNotes(json["notes"].toString());

    if (json.contains("sets"))
    {
        QJsonValue setsVal = json["sets"];
        if (setsVal.isString())
        {
            for (const auto& set : parseSets(setsVal.toString(), errors))
                e.addSet(set);
        }
        else if (setsVal.isArray())
        {
            for (const auto& val : setsVal.toArray())
                e.addSet(setFromJson(val.toObject()));
        }
    }

    return e;
}

Workout workoutFromJson(const QJsonObject& json, QStringList* errors)
{
    Workout w;
    if (json.contains("id"))
        w.setId(json["id"].toInt());
    if (json.contains("name"))
        w.setName(json["name"].toString());
    if (json.contains("created_time"))
        w.setCreatedTime(QDateTime::fromString(json["created_time"].toString(), Qt::ISODate));
    if (json.contains("planned_time"))
        w.setPlannedTime(QDateTime::fromString(json["planned_time"].toString(), Qt::ISODate));
    if (json.contains("started_time"))
        w.setStartedTime(QDateTime::fromString(json["started_time"].toString(), Qt::ISODate));
    if (json.contains("ended_time"))
        w.setEndedTime(QDateTime::fromString(json["ended_time"].toString(), Qt::ISODate));
    if (json.contains("status"))
        w.setStatus(workoutStatusFromString(json["status"].toString()));

    if (json.contains("exercises"))
    {
        for (const auto& val : json["exercises"].toArray())
            w.addExercise(exerciseFromJson(val.toObject(), errors));
    }

    return w;
}

std::vector<Workout> workoutsFromJsonArray(const QJsonArray& array, QStringList* errors)
{
    std::vector<Workout> workouts;
    for (const auto& val : array)
        workouts.push_back(workoutFromJson(val.toObject(), errors));
    return workouts;
}

}  // namespace WorkoutJson
