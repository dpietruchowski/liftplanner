#include "workoutjson.h"
#include <QJsonDocument>

namespace WorkoutJson
{

    QJsonObject setToJson(const Set &set)
    {
        QJsonObject obj;
        if (set.id() != -1)
            obj["id"] = set.id();
        if (set.exerciseId() != -1)
            obj["exercise_id"] = set.exerciseId();
        obj["repetitions"] = set.repetitions();
        obj["weight"] = set.weight();
        obj["completed"] = set.completed();
        return obj;
    }

    QJsonObject exerciseToJson(const Exercise &exercise)
    {
        QJsonObject obj;
        if (exercise.id() != -1)
            obj["id"] = exercise.id();
        if (exercise.workoutId() != -1)
            obj["workout_id"] = exercise.workoutId();
        obj["name"] = exercise.name();
        obj["description"] = exercise.description();
        obj["rest_seconds"] = exercise.restSeconds();
        obj["youtube_link"] = exercise.youtubeLink();

        QJsonArray setsArray;
        for (const auto &set : exercise.sets())
            setsArray.append(setToJson(set));
        obj["sets"] = setsArray;

        return obj;
    }

    QJsonObject workoutToJson(const Workout &workout)
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

        QJsonArray exercisesArray;
        for (const auto &exercise : workout.exercises())
            exercisesArray.append(exerciseToJson(exercise));
        obj["exercises"] = exercisesArray;

        return obj;
    }

    QJsonObject exerciseToJsonCompact(const Exercise &exercise)
    {
        QJsonObject obj;
        obj["name"] = exercise.name();
        obj["rest_seconds"] = exercise.restSeconds();
        obj["youtube_link"] = exercise.youtubeLink();
        obj["sets"] = exercise.setsToString();
        return obj;
    }

    QJsonObject workoutToJsonCompact(const Workout &workout)
    {
        QJsonObject obj;
        obj["name"] = workout.name();

        if (workout.plannedTime().isValid())
            obj["planned_time"] = workout.plannedTime().toString(Qt::ISODate);
        if (workout.startedTime().isValid())
            obj["started_time"] = workout.startedTime().toString(Qt::ISODate);

        QJsonArray exercisesArray;
        for (const auto &exercise : workout.exercises())
            exercisesArray.append(exerciseToJsonCompact(exercise));
        obj["exercises"] = exercisesArray;

        return obj;
    }

    Set setFromJson(const QJsonObject &json)
    {
        Set s;
        if (json.contains("id"))
            s.setId(json["id"].toInt());
        if (json.contains("exercise_id"))
            s.setExerciseId(json["exercise_id"].toInt());
        if (json.contains("repetitions"))
            s.setRepetitions(json["repetitions"].toInt());
        if (json.contains("weight"))
            s.setWeight(json["weight"].toInt());
        if (json.contains("completed"))
            s.setCompleted(json["completed"].toBool());
        return s;
    }

    static std::vector<Set> parseSetsFromString(const QString &str)
    {
        std::vector<Set> sets;
        QStringList parts = str.split(",", Qt::SkipEmptyParts);
        for (const QString &part : parts)
        {
            QString s = part.trimmed().toLower().replace("kg", "");
            QStringList tokens = s.split("x");
            if (tokens.size() == 2)
            {
                bool ok1 = false, ok2 = false;
                int reps = tokens[0].trimmed().toInt(&ok1);
                int weight = tokens[1].trimmed().toInt(&ok2);
                if (ok1 && ok2)
                    sets.emplace_back(reps, weight);
            }
        }
        return sets;
    }

    Exercise exerciseFromJson(const QJsonObject &json)
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
        if (json.contains("youtube_link"))
            e.setYoutubeLink(json["youtube_link"].toString());

        if (json.contains("sets"))
        {
            QJsonValue setsVal = json["sets"];
            if (setsVal.isString())
            {
                for (const auto &set : parseSetsFromString(setsVal.toString()))
                    e.addSet(set);
            }
            else if (setsVal.isArray())
            {
                for (const auto &val : setsVal.toArray())
                    e.addSet(setFromJson(val.toObject()));
            }
        }

        return e;
    }

    Workout workoutFromJson(const QJsonObject &json)
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

        if (json.contains("exercises"))
        {
            for (const auto &val : json["exercises"].toArray())
                w.addExercise(exerciseFromJson(val.toObject()));
        }

        return w;
    }

    std::vector<Workout> workoutsFromJsonArray(const QJsonArray &array)
    {
        std::vector<Workout> workouts;
        for (const auto &val : array)
            workouts.push_back(workoutFromJson(val.toObject()));
        return workouts;
    }

} // namespace WorkoutJson
