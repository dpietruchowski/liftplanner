#include "workouthistoryviewmodel.h"
#include "modules/workout/application/workoutservice.h"
#include "utils/workoutjson.h"
#include "utils/workouttext.h"

WorkoutHistoryViewModel::WorkoutHistoryViewModel(WorkoutService* service, QObject* parent)
    : QObject(parent)
    , m_service(service)
{
    connect(this, &WorkoutHistoryViewModel::workoutsChanged, this,
            &WorkoutHistoryViewModel::lastWorkoutChanged);
    connect(this, &WorkoutHistoryViewModel::workoutsChanged, this,
            &WorkoutHistoryViewModel::topExercisesChanged);

    loadAllWorkouts();
}

void WorkoutHistoryViewModel::loadAllWorkouts()
{
    qDeleteAll(m_workouts);
    m_workouts.clear();

    auto entities = m_service->loadHistory();
    for (const auto& entity : entities)
        m_workouts.append(new WorkoutModel(entity, this));

    emit workoutsChanged();
}

void WorkoutHistoryViewModel::saveWorkout(WorkoutModel* workout)
{
    if (!m_service || !workout)
        return;
    if (!workout->startedTime().isValid())
        return;

    Workout entity = workout->toEntity();
    m_service->saveWorkout(entity);
    loadAllWorkouts();
}

void WorkoutHistoryViewModel::deleteWorkout(WorkoutModel* workout)
{
    if (!m_service || !workout)
        return;

    m_service->deleteWorkout(workout->id());
    loadAllWorkouts();
}

QJsonArray WorkoutHistoryViewModel::recentWorkoutsToJson(int count)
{
    QJsonArray jsonArray;

    int actualCount = qMin(count, m_workouts.size());
    for (int i = 0; i < actualCount; ++i)
    {
        WorkoutModel* workout = m_workouts.at(i);
        if (workout)
            jsonArray.append(WorkoutJson::workoutToJsonCompact(workout->toEntity()));
    }

    return jsonArray;
}

WorkoutModel* WorkoutHistoryViewModel::lastWorkout() const
{
    if (m_workouts.isEmpty())
        return nullptr;

    return m_workouts.first();
}

QVariantList WorkoutHistoryViewModel::topExercises() const
{
    QVariantList result;
    if (!m_service)
        return result;

    for (const auto& entry : m_service->topExercises(2, 20))
    {
        QVariantMap item;
        item["name"] = entry.name;
        item["count"] = entry.count;
        item["oneRepMax"] = entry.bestOneRepMax;
        result.append(item);
    }

    return result;
}

void WorkoutHistoryViewModel::importFromJson(const QString& jsonData)
{
    try
    {
        QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
        if (doc.isNull() || !doc.isArray())
        {
            emit errorOccurred("Import failed: invalid JSON");
            return;
        }

        auto workouts = WorkoutJson::workoutsFromJsonArray(doc.array());
        m_service->importHistory(workouts);
        loadAllWorkouts();
    }
    catch (const std::exception& e)
    {
        emit errorOccurred(QString("Import failed: %1").arg(e.what()));
    }
}

void WorkoutHistoryViewModel::importFromClipboard()
{
    try
    {
        QClipboard* clipboard = QGuiApplication::clipboard();
        QString jsonData = clipboard->text();

        if (jsonData.isEmpty())
        {
            emit errorOccurred("Clipboard is empty");
            return;
        }

        importFromJson(jsonData);
    }
    catch (const std::exception& e)
    {
        emit errorOccurred(QString("Import from clipboard failed: %1").arg(e.what()));
    }
}

void WorkoutHistoryViewModel::exportToClipboard(int limit)
{
    try
    {
        QJsonArray jsonArray;
        int count = limit > 0 ? qMin(limit, m_workouts.size()) : m_workouts.size();
        for (int i = 0; i < count; ++i)
            jsonArray.append(WorkoutJson::workoutToJsonCompact(m_workouts.at(i)->toEntity()));

        QJsonDocument doc(jsonArray);
        QString jsonString = doc.toJson(QJsonDocument::Indented);

        QClipboard* clipboard = QGuiApplication::clipboard();
        clipboard->setText(jsonString);

        emit exportedToClipboard();
    }
    catch (const std::exception& e)
    {
        emit errorOccurred(QString("Export failed: %1").arg(e.what()));
    }
}

void WorkoutHistoryViewModel::exportWorkoutToClipboard(WorkoutModel* workout)
{
    if (!workout)
        return;

    try
    {
        QClipboard* clipboard = QGuiApplication::clipboard();
        clipboard->setText(WorkoutText::workoutToText(workout->toEntity()));

        emit exportedToClipboard();
    }
    catch (const std::exception& e)
    {
        emit errorOccurred(QString("Export failed: %1").arg(e.what()));
    }
}
