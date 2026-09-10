#include "workouthistoryviewmodel.h"
#include "application/workout/workoutservice.h"
#include "async/timeprovider.h"
#include "domain/workout/performedsets.h"
#include "domain/workout/sessionsummary.h"
#include "infrastructure/workout/workoutjson.h"
#include "ui/presentation/historyimportcheck.h"
#include "ui/presentation/workouttext.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include <QDate>

namespace
{
constexpr int top_exercise_tiles = 2;
constexpr int recent_stats_window = 20;

QVariantMap statTile(const QString& label, const QString& value)
{
    QVariantMap tile;
    tile["label"] = label;
    tile["value"] = value;
    return tile;
}
}  // namespace

WorkoutHistoryViewModel::WorkoutHistoryViewModel(WorkoutService* service,
                                                 ActiveWorkoutViewModel* activeWorkoutViewModel,
                                                 QObject* parent)
    : QObject(parent)
    , m_service(service)
    , m_activeWorkoutViewModel(activeWorkoutViewModel)
{
    connect(this, &WorkoutHistoryViewModel::workoutsChanged, this,
            &WorkoutHistoryViewModel::lastWorkoutChanged);
    connect(this, &WorkoutHistoryViewModel::workoutsChanged, this,
            &WorkoutHistoryViewModel::weekActivityChanged);

    if (m_activeWorkoutViewModel)
    {
        connect(m_activeWorkoutViewModel, &ActiveWorkoutViewModel::currentWorkoutChanged, this,
                &WorkoutHistoryViewModel::weekActivityChanged);
        connect(m_activeWorkoutViewModel, &ActiveWorkoutViewModel::workoutCompleted, this,
                &WorkoutHistoryViewModel::loadAllWorkouts);
        connect(m_activeWorkoutViewModel, &ActiveWorkoutViewModel::interruptedWorkoutSettled, this,
                &WorkoutHistoryViewModel::loadAllWorkouts);
    }

    loadAllWorkouts();
}

bool WorkoutHistoryViewModel::isLoading() const { return m_loading; }

void WorkoutHistoryViewModel::setLoading(bool value)
{
    if (m_loading == value)
        return;

    m_loading = value;
    emit loadingChanged();
}

void WorkoutHistoryViewModel::loadAllWorkouts()
{
    if (!m_service)
        return;

    setLoading(true);

    m_service->loadHistory()
        .then(this,
              [this](std::vector<Workout> entities)
              {
                  qDeleteAll(m_workouts);
                  m_workouts.clear();
                  for (const auto& entity : entities)
                      m_workouts.append(new WorkoutModel(entity, this));

                  setLoading(false);
                  emit workoutsChanged();
                  refreshTopExercises();
                  refreshRecentTotals();
              })
        .onError(this,
                 [this](const QString& error)
                 {
                     setLoading(false);
                     emit errorOccurred(error);
                 });
}

void WorkoutHistoryViewModel::refreshTopExercises()
{
    if (!m_service)
        return;

    m_service->topExercises(top_exercise_tiles, recent_stats_window)
        .then(this,
              [this](std::vector<WorkoutService::ExerciseFrequency> entries)
              {
                  m_topExercises.clear();
                  for (const auto& entry : entries)
                  {
                      QVariantMap item;
                      item["name"] = entry.name;
                      item["count"] = entry.count;
                      item["oneRepMax"] = entry.bestOneRepMax;
                      m_topExercises.append(item);
                  }
                  emit topExercisesChanged();
              })
        .warnOnError("load the top exercises");
}

void WorkoutHistoryViewModel::refreshRecentTotals()
{
    if (!m_service)
        return;

    m_service->recentTotals(recent_stats_window)
        .then(this,
              [this](WorkoutService::TrainingTotals totals)
              {
                  m_recentTotals.clear();

                  if (totals.totalWeight > 0.0)
                      m_recentTotals.append(
                          statTile("volume", WorkoutText::formatVolume(totals.totalWeight)));

                  if (totals.totalDurationSeconds > 0)
                      m_recentTotals.append(statTile(
                          "time", WorkoutText::formatDuration(totals.totalDurationSeconds)));

                  if (totals.totalDistanceMeters > 0.0)
                      m_recentTotals.append(statTile(
                          "distance", WorkoutText::formatDistance(totals.totalDistanceMeters)));

                  emit recentTotalsChanged();
              })
        .warnOnError("load the recent totals");
}

void WorkoutHistoryViewModel::saveWorkout(WorkoutModel* workout)
{
    if (!m_service || !workout)
        return;
    if (!workout->startedTime().isValid())
        return;

    m_service->saveWorkout(workout->toEntity()).warnOnError("save the workout");
    loadAllWorkouts();
}

void WorkoutHistoryViewModel::deleteWorkout(WorkoutModel* workout)
{
    if (!m_service || !workout)
        return;

    m_service->deleteWorkout(workout->id()).warnOnError("delete the workout");
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

QVariantList WorkoutHistoryViewModel::weekActivity() const
{
    QVariantList activity;
    for (int i = 0; i < 7; ++i)
        activity.append(false);

    const QDate today = TimeProvider::instance().currentDate();
    const QDate monday = today.addDays(1 - today.dayOfWeek());

    const auto markDay = [&](const WorkoutModel* workout)
    {
        if (!workout || !workout->startedTime().isValid())
            return;
        const int dayIndex = static_cast<int>(monday.daysTo(workout->startedTime().date()));
        if (dayIndex >= 0 && dayIndex < 7)
            activity[dayIndex] = true;
    };

    for (const auto* workout : m_workouts)
        markDay(workout);

    if (m_activeWorkoutViewModel)
        markDay(m_activeWorkoutViewModel->currentWorkout());

    return activity;
}

QVariantList WorkoutHistoryViewModel::topExercises() const { return m_topExercises; }

QVariantList WorkoutHistoryViewModel::recentTotals() const { return m_recentTotals; }

WorkoutModel* WorkoutHistoryViewModel::ownerOf(SetModel* set) const
{
    auto* exercise = set ? qobject_cast<ExerciseModel*>(set->parent()) : nullptr;
    return exercise ? qobject_cast<WorkoutModel*>(exercise->parent()) : nullptr;
}

void WorkoutHistoryViewModel::requestSetToggle(SetModel* set)
{
    WorkoutModel* workout = ownerOf(set);
    if (!workout)
        return;

    m_pendingToggle.clear();

    const Workout entity = workout->toEntity();
    if (set->completed() || completionFlagsAreMeaningful(entity))
    {
        applyToggle(set);
        return;
    }

    m_pendingToggle = set;
    emit amnestyWarningRaised(WorkoutText::amnestyLossWarning(
        entity.name(), static_cast<int>(summarizeSession(entity).plannedSets)));
}

void WorkoutHistoryViewModel::confirmPendingToggle()
{
    SetModel* set = m_pendingToggle.data();
    m_pendingToggle.clear();
    applyToggle(set);
}

void WorkoutHistoryViewModel::cancelPendingToggle() { m_pendingToggle.clear(); }

void WorkoutHistoryViewModel::applyToggle(SetModel* set)
{
    WorkoutModel* workout = ownerOf(set);
    if (!m_service || !workout)
        return;

    set->setCompleted(!set->completed());

    m_service->saveWorkout(workout->toEntity())
        .then(this,
              [this](int)
              {
                  refreshTopExercises();
                  refreshRecentTotals();
                  emit lastWorkoutChanged();
              })
        .onError(this, [this](const QString& error) { emit errorOccurred(error); });
}

void WorkoutHistoryViewModel::importFromJson(const QString& jsonData)
{
    const QString problem = HistoryImportCheck::payloadProblem(jsonData);
    if (!problem.isEmpty())
    {
        emit errorOccurred(problem);
        return;
    }

    if (!m_service)
        return;

    const QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
    const auto workouts = WorkoutJson::workoutsFromJsonArray(doc.array());

    m_service->importHistory(workouts)
        .then(this, [this] { loadAllWorkouts(); })
        .onError(this, [this](const QString& error)
                 { emit errorOccurred(QStringLiteral("Import failed: %1").arg(error)); });
}

void WorkoutHistoryViewModel::importFromClipboard()
{
    const QString clipboardText = QGuiApplication::clipboard()->text();

    const QString problem = HistoryImportCheck::clipboardProblem(clipboardText);
    if (!problem.isEmpty())
    {
        emit errorOccurred(problem);
        return;
    }

    importFromJson(clipboardText);
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
