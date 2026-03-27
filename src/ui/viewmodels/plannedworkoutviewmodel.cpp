#include "plannedworkoutviewmodel.h"
#include <QClipboard>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include "modules/workout/application/workoutservice.h"
#include "utils/workoutjson.h"

PlannedWorkoutViewModel::PlannedWorkoutViewModel(WorkoutService *service, QObject *parent)
    : QObject(parent), m_service(service)
{
}

PlannedWorkoutViewModel::~PlannedWorkoutViewModel()
{
    qDeleteAll(m_workouts);
    m_workouts.clear();
}

QList<WorkoutModel *> PlannedWorkoutViewModel::workouts() const
{
    return m_workouts;
}

WorkoutModel *PlannedWorkoutViewModel::nextWorkout() const
{
    return m_workouts.isEmpty() ? nullptr : m_workouts.first();
}

void PlannedWorkoutViewModel::loadAll()
{
    try
    {
        qDeleteAll(m_workouts);
        m_workouts.clear();

        auto entities = m_service->loadPlannedWorkouts();
        for (const auto &entity : entities)
            m_workouts.append(new WorkoutModel(entity, this));

        emit workoutsChanged();
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("Failed to load planned workouts: %1").arg(e.what()));
    }
}

void PlannedWorkoutViewModel::importFromJson(const QString &jsonData)
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
        auto workouts = WorkoutJson::workoutsFromJsonArray(doc.array());

        QDateTime baseTime = QDateTime::currentDateTime();
        for (size_t i = 0; i < workouts.size(); ++i)
        {
            if (!workouts[i].plannedTime().isValid())
                workouts[i].setPlannedTime(baseTime.addDays(static_cast<qint64>(i + 1)));
        }

        m_service->importPlannedWorkouts(workouts);
        loadAll();
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("Import failed: %1").arg(e.what()));
    }
}

void PlannedWorkoutViewModel::importFromClipboard()
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

        importFromJson(jsonData);
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("Import from clipboard failed: %1").arg(e.what()));
    }
}

void PlannedWorkoutViewModel::generatePrompt()
{
    QString prompt = "You are a personal training assistant. Your role is to help the user "
                     "plan their workouts based on their goals, history, and current plan.\n\n";

    auto history = m_service->loadHistory();

    if (!history.empty())
    {
        QJsonArray historyArray;
        int count = std::min(10, static_cast<int>(history.size()));
        for (int i = 0; i < count; ++i)
            historyArray.append(WorkoutJson::workoutToJsonCompact(history[i]));

        prompt += "=== RECENT WORKOUT HISTORY (last " + QString::number(count) + ") ===\n" + QJsonDocument(historyArray).toJson(QJsonDocument::Indented) + "\n";
    }

    if (!m_workouts.isEmpty())
    {
        QJsonArray plannedArray;
        for (auto *w : m_workouts)
            plannedArray.append(WorkoutJson::workoutToJsonCompact(w->toEntity()));

        prompt += "=== CURRENTLY PLANNED WORKOUTS ===\n" + QJsonDocument(plannedArray).toJson(QJsonDocument::Indented) + "\n" + "Review the planned workouts above. Based on the training history, "
                                                                                                                                "evaluate the plan and suggest improvements or changes. "
                                                                                                                                "Discuss with the user before generating the final plan.\n\n";
    }
    else if (history.empty())
    {
        prompt += "The user has no workout history and no planned workouts yet.\n"
                  "Conduct a brief interview to gather:\n"
                  "- Gender, age, weight\n"
                  "- Training goals (strength, hypertrophy, endurance, etc.)\n"
                  "- Experience level\n"
                  "- How many days per week\n"
                  "- Available equipment (gym, home, dumbbells only, etc.)\n"
                  "- Any injuries or limitations\n"
                  "- Available time per workout\n\n";
    }
    else
    {
        prompt += "The user has workout history but no planned workouts.\n"
                  "Based on the history above, suggest a new training plan. "
                  "Discuss with the user before generating.\n\n";
    }

    QString templateContent = readTemplateFile(":/LiftPlanner/data/gpt_prompt_template.txt");
    if (!templateContent.isEmpty())
        prompt += templateContent;

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(prompt);
    emit promptGenerated();
}

bool PlannedWorkoutViewModel::validateJson(const QString &jsonData, QString &errorMessage)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        errorMessage = QString("Invalid JSON: %1").arg(parseError.errorString());
        return false;
    }

    if (!doc.isArray())
    {
        errorMessage = "JSON is not an array of workouts";
        return false;
    }

    QJsonArray workoutsArray = doc.array();
    int index = 0;
    for (const QJsonValue &workoutValue : workoutsArray)
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
                errorMessage = QString("Workout at index %1: 'exercises' is not an array").arg(index);
                return false;
            }

            QJsonArray exercisesArray = workoutObj["exercises"].toArray();
            for (int j = 0; j < exercisesArray.size(); ++j)
            {
                if (!exercisesArray[j].isObject())
                {
                    errorMessage = QString("Workout %1, exercise %2: not a valid object").arg(index).arg(j);
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

QString PlannedWorkoutViewModel::readTemplateFile(const QString &filePath)
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
