#include "routineservice.h"
#include <QClipboard>
#include <QDebug>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>

RoutineService::RoutineService(AppDbStorage *dbStorage, QObject *parent)
    : QObject(parent), m_dbStorage(dbStorage)
{
}

RoutineService::~RoutineService()
{
    qDeleteAll(m_workouts);
    m_workouts.clear();
}

QList<QObject *> RoutineService::workouts() const
{
    return m_workouts;
}

void RoutineService::loadAllWorkouts()
{
    try
    {
        qDeleteAll(m_workouts);
        m_workouts.clear();

        QVector<WorkoutModel *> workoutModels = m_dbStorage->workoutRepository()->loadAll(
            "WHERE started_time IS NULL");

        for (WorkoutModel *workout : workoutModels)
        {
            m_dbStorage->loadWorkout(workout);
            m_workouts.append(workout);
        }

        emit workoutsChanged();
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("Failed to load workouts: %1").arg(e.what()));
    }
}

void RoutineService::importWorkoutsFromJson(const QString &jsonData)
{
    QString validationError;
    if (!validateWorkoutsJson(jsonData, validationError)) {
        emit errorOccurred(QString("Import failed: %1").arg(validationError));
        return;
    }

    try {
        m_dbStorage->workoutRepository()->remove("WHERE started_time IS NULL");
        QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
        if (doc.isArray())
        {
            QJsonArray workoutsArray = doc.array();
            for (const QJsonValue &workoutValue : workoutsArray)
            {
                WorkoutModel *workout = WorkoutModel::fromJson(workoutValue.toObject());
                if (workout)
                {
                    m_dbStorage->saveWorkout(workout);
                    delete workout;
                }
            }
            loadAllWorkouts();
        }
    } catch (const std::exception &e) {
        auto errorStr = QString("Import failed: %1").arg(e.what());
        emit errorOccurred(errorStr);
    }
}

void RoutineService::generateGptPrompt(const QJsonArray &recentTrainings)
{
    QString prompt;

    if (recentTrainings.isEmpty()) {
        prompt += R"(
You are a personal training assistant. Gather details from the user and generate a beginner workout plan.
Ask for:
- Gender (male/female)
- Age
- Weight
- Goal
- Training experience
- Workouts per week
- Equipment
- Injuries or limitations
- Time per workout
)";
    } else {
        prompt += R"(
You are a personal training assistant. Analyze the user's recent workouts, evaluate their progress, and provide brief suggestions on what to adjust.
Include the user's recent workouts as reference:
)";

        QString recentTrainingsString = QJsonDocument(recentTrainings)
                                            .toJson(QJsonDocument::Indented);
        prompt += recentTrainingsString;
    }

    prompt += R"(

Step 1: Show the user the proposed exercises with sets in the format repsxweightkg and wait for confirmation.

Step 2: After confirmation, generate the full workout plan as valid JSON wrapped in a code block.

Format of the JSON:
- The top-level object is a list of workouts.
- Each workout has:
  - "name": string
  - "exercises": list of exercises
- Each exercise has:
  - "name": string
  - "description": string
  - "sets": string with each set in format repsxweightkg, separated by commas (e.g., "10x50kg, 8x55kg, 6x60kg")
- Example structure (without actual example values):
[
  {
    "name": "Workout Name",
    "exercises": [
      {
        "name": "Exercise Name",
        "description": "Instructions",
        "sets": "10x50kg, 8x55kg"
      }
    ]
  }
]

Rules:
- Each set must always be in the format repsxweightkg (e.g., 10x50kg, 8x0kg for bodyweight exercises).
- Weights in kg, repetitions as integers.
- Ensure the workouts are balanced and appropriate for the user.
)";

    prompt += "\n";

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(prompt);
}

void RoutineService::importWorkoutsFromClipboard()
{
    try
    {
        QClipboard *clipboard = QGuiApplication::clipboard();
        QString jsonData = clipboard->text();

        if (jsonData.isEmpty())
        {
            emit errorOccurred("Clipboard is empty");
            return;
        }

        importWorkoutsFromJson(jsonData);
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("Import from clipboard failed: %1").arg(e.what()));
    }
}

bool RoutineService::validateWorkoutsJson(const QString &jsonData, QString &errorMessage)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = QString("Invalid JSON: %1").arg(parseError.errorString());
        return false;
    }

    if (!doc.isArray()) {
        errorMessage = "JSON is not an array of workouts";
        return false;
    }

    QJsonArray workoutsArray = doc.array();
    int index = 0;
    for (const QJsonValue &workoutValue : workoutsArray) {
        if (!workoutValue.isObject()) {
            errorMessage = QString("Workout at index %1 is not a JSON object").arg(index);
            return false;
        }

        QVariantMap workoutMap = Serialization::fromJson(workoutValue.toObject());
        QString workoutError;
        if (!WorkoutModel::validateVariantMap(workoutMap, workoutError)) {
            errorMessage = QString("Workout at index %1 invalid: %2").arg(index).arg(workoutError);
            return false;
        }

        index++;
    }

    errorMessage.clear();
    return true;
}
