#include "workoutservice.h"
#include <QClipboard>
#include <QDebug>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>

WorkoutService::WorkoutService(AppDbStorage *dbStorage, QObject *parent)
    : QObject(parent)
    , m_dbStorage(dbStorage)
{
}

WorkoutService::~WorkoutService()
{
    qDeleteAll(m_workouts);
    m_workouts.clear();
}

QList<QObject*> WorkoutService::workouts() const
{
    return m_workouts;
}

void WorkoutService::loadAllWorkouts()
{
    try {
        qDeleteAll(m_workouts);
        m_workouts.clear();

        QVector<WorkoutModel*> workoutModels = m_dbStorage->workoutRepository()->loadAll();
        
        for (WorkoutModel *workout : workoutModels) {
            loadExercisesForWorkout(workout);
            m_workouts.append(workout);
        }

        emit workoutsChanged();

    } catch (const std::exception &e) {
        emit errorOccurred(QString("Failed to load workouts: %1").arg(e.what()));
    }
}

void WorkoutService::importWorkoutsFromJson(const QString &jsonData)
{
    try {
        m_dbStorage->workoutRepository()->remove("");
        QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
        if (doc.isArray()) {
            QJsonArray workoutsArray = doc.array();
            for (const QJsonValue &workoutValue : workoutsArray) {
                WorkoutModel *workout = WorkoutModel::fromJson(workoutValue.toObject());
                if (workout) {
                    m_dbStorage->saveWorkout(workout);
                    delete workout;
                }
            }
            loadAllWorkouts();
        }
    } catch (const std::exception &e) {
        emit errorOccurred(QString("Import failed: %1").arg(e.what()));
    }
}

void WorkoutService::generateGptPrompt()
{
    QString prompt = R"(
You are a personal training assistant. Your task is to first gather details from the user, then generate a gym workout plan in JSON format.

Follow these steps:

### Step 1: Ask questions
Ask the user these questions one by one:
1. Choose language
2. What is your **main goal**? (Build muscle, lose fat, strength, endurance)
3. What is your **training experience**? (Beginner, Intermediate, Advanced)
4. How many **workouts per week** can you do? (e.g., 3, 4, 5)
5. What **equipment** do you have access to? (Full gym, dumbbells only, bodyweight)
6. Any **injuries or limitations**? (Shoulder, knee, back, etc.)
7. How much **time per workout** do you have? (e.g., 45 min, 60 min)

Be friendly and clear, and confirm the user’s answers before generating the plan.

### Step 2: Explain output format with an example
Show this short example so the user understands what the result will look like:
```
[
  {
    "name": "Trening A",
    "exercises": [
      {
        "name": "Bench Press",
        "description: "How to bench press"
        "sets": [
          {"repetitions": 10, "weight": 50},
          {"repetitions": 8, "weight": 55},
          {"repetitions": 6, "weight": 60}
        ]
      }
    ]
  }
]
```

### Step 3: Generate the workout plan
Once the user provides all answers, create a plan following these rules:
- Return ONLY valid JSON, no extra text.
- Use the exact structure from the example.
- Simple very short names for exercises
- Each workout should have **3-5 exercises** arranged in the correct order:
  - Start with compound lifts (e.g., squats, bench press, deadlift)
  - Then assistance and isolation exercises (e.g., biceps curls, lateral raises)
- Each exercise should have **3-5 sets**, with realistic progression of weights or fixed weights.
- Weights in kg, repetitions as integers.
- Make the program logical, progressive, and aligned with the user's goals and experience.
- Ensure the workouts are balanced and fit within the user’s time constraints.

Wait for the user’s answers and confirmation before generating the JSON.
)";

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(prompt);
}

void WorkoutService::importWorkoutsFromClipboard()
{
    try {
        QClipboard *clipboard = QGuiApplication::clipboard();
        QString jsonData = clipboard->text();

        if (jsonData.isEmpty()) {
            emit errorOccurred("Clipboard is empty");
            return;
        }

        importWorkoutsFromJson(jsonData);

    } catch (const std::exception &e) {
        emit errorOccurred(QString("Import from clipboard failed: %1").arg(e.what()));
    }
}

void WorkoutService::loadExercisesForWorkout(WorkoutModel *workout)
{
    if (!workout) return;

    QVector<ExerciseModel *> exercises = m_dbStorage->exerciseRepository()->loadAll(
        QString("WHERE workout_id = %1").arg(workout->id()));

    for (ExerciseModel *exercise : exercises) {
        loadSetsForExercise(exercise);
        workout->addExercise(exercise);
    }
}

void WorkoutService::loadSetsForExercise(ExerciseModel *exercise)
{
    if (!exercise) return;

    QVector<SetModel *> sets = m_dbStorage->setRepository()->loadAll(
        QString("WHERE exercise_id = %1").arg(exercise->id()));

    for (SetModel *set : sets) {
        exercise->addSet(set);
    }
}
