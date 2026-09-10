#include "plannedworkoutviewmodel.h"
#include "application/userprofile/userprofileservice.h"
#include "application/workout/workoutservice.h"
#include "async/timeprovider.h"
#include "infrastructure/userprofile/userprofileserializer.h"
#include "infrastructure/workout/workoutjson.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include <QClipboard>
#include <QDate>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

PlannedWorkoutViewModel::PlannedWorkoutViewModel(WorkoutService* service,
                                                 UserProfileService* profileService,
                                                 ActiveWorkoutViewModel* activeWorkoutViewModel,
                                                 QObject* parent)
    : QObject(parent)
    , m_service(service)
    , m_profileService(profileService)
{
    if (activeWorkoutViewModel)
    {
        connect(activeWorkoutViewModel, &ActiveWorkoutViewModel::workoutDiscarded, this,
                &PlannedWorkoutViewModel::loadAll);
        connect(activeWorkoutViewModel, &ActiveWorkoutViewModel::interruptedWorkoutSettled, this,
                &PlannedWorkoutViewModel::loadAll);
    }
}

PlannedWorkoutViewModel::~PlannedWorkoutViewModel()
{
    qDeleteAll(m_workouts);
    m_workouts.clear();
}

QList<WorkoutModel*> PlannedWorkoutViewModel::workouts() const { return m_workouts; }

WorkoutModel* PlannedWorkoutViewModel::nextWorkout() const
{
    return m_workouts.isEmpty() ? nullptr : m_workouts.first();
}

bool PlannedWorkoutViewModel::isLoading() const { return m_loading; }

void PlannedWorkoutViewModel::setLoading(bool value)
{
    if (m_loading == value)
        return;

    m_loading = value;
    emit loadingChanged();
}

void PlannedWorkoutViewModel::loadAll()
{
    if (!m_service)
        return;

    setLoading(true);

    m_service->loadPlannedWorkouts()
        .then(this,
              [this](std::vector<Workout> entities)
              {
                  qDeleteAll(m_workouts);
                  m_workouts.clear();
                  for (const auto& entity : entities)
                      m_workouts.append(new WorkoutModel(entity, this));

                  setLoading(false);
                  emit workoutsChanged();
              })
        .onError(this,
                 [this](const QString& error)
                 {
                     setLoading(false);
                     emit errorOccurred(error);
                 });
}

void PlannedWorkoutViewModel::repeatWorkout(WorkoutModel* workout)
{
    if (!m_service || !workout || workout->id() <= 0)
        return;

    setLoading(true);

    const QString name = workout->name();

    m_service->repeatWorkout(workout->id(), TimeProvider::instance().currentDateTime())
        .then(this,
              [this, name](int)
              {
                  loadAll();
                  emit workoutRepeated(name);
              })
        .onError(this,
                 [this](const QString& error)
                 {
                     setLoading(false);
                     emit errorOccurred(error);
                 });
}

void PlannedWorkoutViewModel::deleteWorkout(WorkoutModel* workout)
{
    if (!m_service || !workout || workout->id() <= 0)
        return;

    setLoading(true);

    m_service->deleteWorkout(workout->id())
        .then(this, [this](bool) { loadAll(); })
        .onError(this,
                 [this](const QString& error)
                 {
                     setLoading(false);
                     emit errorOccurred(error);
                 });
}

void PlannedWorkoutViewModel::importFromJson(const QString& jsonData)
{
    QString validationError;
    if (!validateJson(jsonData, validationError))
    {
        emit errorOccurred(QString("Import failed: %1").arg(validationError));
        return;
    }

    try
    {
        QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
        QJsonObject root = doc.object();

        QJsonValue profileValue = root.value("user_profile");
        if (m_profileService && profileValue.isObject())
        {
            QVariantMap vm = profileValue.toObject().toVariantMap();
            vm[UserProfileSerializer::user_id_key] = 1;
            m_profileService->save(UserProfileSerializer::fromVariant(vm))
                .warnOnError("save the imported user profile");
        }

        QJsonArray workoutsArray = root.value("workouts").toArray();
        QStringList setErrors;
        auto workouts = WorkoutJson::workoutsFromJsonArray(workoutsArray, &setErrors);

        QDateTime baseTime = TimeProvider::instance().currentDateTime();
        for (size_t i = 0; i < workouts.size(); ++i)
        {
            if (!workouts[i].plannedTime().isValid())
                workouts[i].setPlannedTime(baseTime.addDays(static_cast<qint64>(i + 1)));
        }

        m_service->importPlannedWorkouts(workouts).warnOnError("import planned workouts");
        loadAll();

        if (!setErrors.isEmpty())
            emit errorOccurred(QString("Imported, but some sets were skipped: %1")
                                   .arg(summarizeErrors(setErrors)));
    }
    catch (const std::exception& e)
    {
        emit errorOccurred(QString("Import failed: %1").arg(e.what()));
    }
}

void PlannedWorkoutViewModel::importFromClipboard()
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

void PlannedWorkoutViewModel::generatePrompt()
{
    if (!m_service)
        return;

    QString prompt = readTemplateFile(":/LiftPlanner/data/gpt_prompt_template.txt");
    if (prompt.isEmpty())
        return;

    auto finish = [this](QString prompt, QString profileJson)
    {
        m_service->loadHistory()
            .then(this,
                  [this, prompt, profileJson](std::vector<Workout> history) mutable
                  {
                      QJsonArray historyArray;
                      int count = std::min(9, static_cast<int>(history.size()));
                      for (int i = 0; i < count; ++i)
                          historyArray.append(WorkoutJson::workoutToJsonCompact(history[i]));

                      QJsonArray plannedArray;
                      for (auto* w : m_workouts)
                          plannedArray.append(WorkoutJson::workoutToJsonCompact(w->toEntity()));

                      prompt.replace("{{CURRENT_DATE}}",
                                     TimeProvider::instance().currentDate().toString("yyyy-MM-dd"));
                      prompt.replace("{{USER_PROFILE}}", profileJson);
                      prompt.replace("{{HISTORY_JSON}}",
                                     QJsonDocument(historyArray).toJson(QJsonDocument::Indented));
                      prompt.replace("{{PLANNED_JSON}}",
                                     QJsonDocument(plannedArray).toJson(QJsonDocument::Indented));

                      QClipboard* clipboard = QGuiApplication::clipboard();
                      clipboard->setText(prompt);
                      emit promptGenerated();
                  })
            .warnOnError("read the workout history for the prompt");
    };

    if (!m_profileService)
    {
        finish(prompt, "null");
        return;
    }

    m_profileService->load()
        .then(this,
              [prompt, finish](std::optional<UserProfile> profileOpt) mutable
              {
                  QString profileJson = "null";
                  if (profileOpt.has_value())
                  {
                      QVariantMap vm = UserProfileSerializer::toVariant(*profileOpt);
                      vm.remove(UserProfileSerializer::user_id_key);
                      profileJson = QJsonDocument(QJsonObject::fromVariantMap(vm))
                                        .toJson(QJsonDocument::Indented);
                  }
                  finish(prompt, profileJson);
              })
        .warnOnError("read the user profile for the prompt");
}

QString PlannedWorkoutViewModel::summarizeErrors(const QStringList& errors)
{
    constexpr int max_listed = 3;

    if (errors.size() <= max_listed)
        return errors.join(QStringLiteral("; "));

    return QStringLiteral("%1; (+%2 more)")
        .arg(errors.mid(0, max_listed).join(QStringLiteral("; ")))
        .arg(errors.size() - max_listed);
}

bool PlannedWorkoutViewModel::validateJson(const QString& jsonData, QString& errorMessage)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        errorMessage = QString("Invalid JSON: %1").arg(parseError.errorString());
        return false;
    }

    if (!doc.isObject())
    {
        errorMessage = "JSON must be an object with 'user_profile' and 'workouts' keys";
        return false;
    }

    QJsonObject root = doc.object();
    if (!root.contains("workouts") || !root.value("workouts").isArray())
    {
        errorMessage = "JSON missing 'workouts' array";
        return false;
    }

    QJsonArray workoutsArray = root.value("workouts").toArray();
    int index = 0;
    for (const QJsonValue& workoutValue : workoutsArray)
    {
        if (!workoutValue.isObject())
        {
            errorMessage = QString("Workout at index %1 is not a JSON object").arg(index);
            return false;
        }

        QJsonObject workoutObj = workoutValue.toObject();
        if (!workoutObj.contains("name") || !workoutObj["name"].isString())
        {
            errorMessage = QString("Workout at index %1: missing or invalid 'name'").arg(index);
            return false;
        }

        if (workoutObj.contains("exercises"))
        {
            if (!workoutObj["exercises"].isArray())
            {
                errorMessage
                    = QString("Workout at index %1: 'exercises' is not an array").arg(index);
                return false;
            }

            QJsonArray exercisesArray = workoutObj["exercises"].toArray();
            for (int j = 0; j < exercisesArray.size(); ++j)
            {
                if (!exercisesArray[j].isObject())
                {
                    errorMessage
                        = QString("Workout %1, exercise %2: not a valid object").arg(index).arg(j);
                    return false;
                }

                QJsonObject exObj = exercisesArray[j].toObject();
                if (!exObj.contains("name") || !exObj["name"].isString())
                {
                    errorMessage = QString("Workout %1, exercise %2: missing or invalid 'name'")
                                       .arg(index)
                                       .arg(j);
                    return false;
                }
            }
        }

        index++;
    }

    errorMessage.clear();
    return true;
}

QString PlannedWorkoutViewModel::readTemplateFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Cannot open template file:" << filePath;
        return QString();
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    return content;
}
