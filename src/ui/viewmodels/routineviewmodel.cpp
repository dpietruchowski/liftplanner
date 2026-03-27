#include "routineviewmodel.h"
#include <QClipboard>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include "utils/workoutjson.h"
#include "modules/workout/domain/repositories/workoutquery.h"

namespace
{

    QString parseTemplate(const QString &templateContent,
                          bool recentTrainingsEmpty,
                          const QString &recentTrainingsJson = QString())
    {
        QString result = templateContent;

        QRegularExpression
            ifPattern(R"(\{% if recent_trainings_empty %\}(.*?)\{% else %\}(.*?)\{% endif %\})",
                      QRegularExpression::DotMatchesEverythingOption);

        QRegularExpressionMatch match = ifPattern.match(result);
        if (match.hasMatch())
        {
            QString trueBlock = match.captured(1).trimmed();
            QString falseBlock = match.captured(2).trimmed();

            if (recentTrainingsEmpty)
            {
                result.replace(match.captured(0), trueBlock);
            }
            else
            {
                result.replace(match.captured(0), falseBlock);
            }
        }

        if (!recentTrainingsEmpty && !recentTrainingsJson.isEmpty())
        {
            result.replace("{{recent_trainings_json}}", recentTrainingsJson);
        }
        else
        {
            result.replace("{{recent_trainings_json}}", "");
        }

        return result;
    }

    QString readTemplateFile(const QString &filePath)
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

} // namespace

RoutineViewModel::RoutineViewModel(AppDbStorage *dbStorage, QObject *parent)
    : QObject(parent), m_dbStorage(dbStorage)
{
    connect(this, &RoutineViewModel::workoutsChanged, this, &RoutineViewModel::nextWorkoutChanged);
    connect(this,
            &RoutineViewModel::lastWorkoutNameChanged,
            this,
            &RoutineViewModel::nextWorkoutChanged);
}

RoutineViewModel::~RoutineViewModel()
{
    qDeleteAll(m_workouts);
    m_workouts.clear();
}

QList<WorkoutModel *> RoutineViewModel::workouts() const
{
    return m_workouts;
}

void RoutineViewModel::loadAllWorkouts()
{
    try
    {
        qDeleteAll(m_workouts);
        m_workouts.clear();

        WorkoutQuery query;
        query.whereStartedTimeIsNull();

        auto entities = m_dbStorage->loadWorkouts(query);
        for (const auto &entity : entities)
            m_workouts.append(new WorkoutModel(entity, this));

        emit workoutsChanged();
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("Failed to load workouts: %1").arg(e.what()));
    }
}

void RoutineViewModel::importWorkoutsFromJson(const QString &jsonData)
{
    QString validationError;
    if (!validateWorkoutsJson(jsonData, validationError))
    {
        emit errorOccurred(QString("Import failed: %1").arg(validationError));
        return;
    }

    try
    {
        WorkoutQuery removeQuery;
        removeQuery.whereStartedTimeIsNull();
        m_dbStorage->removeWorkoutsByQuery(removeQuery);

        QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
        auto workouts = WorkoutJson::workoutsFromJsonArray(doc.array());
        for (const auto &workout : workouts)
            m_dbStorage->saveWorkout(workout);

        loadAllWorkouts();
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("Import failed: %1").arg(e.what()));
    }
}

void RoutineViewModel::generateGptPrompt(const QJsonArray &recentTrainings)
{
    QString templateContent = readTemplateFile(":/LiftPlanner/data/gpt_prompt_template.txt");

    if (templateContent.isEmpty())
    {
        qWarning() << "GPT template content is empty";
        return;
    }

    QString recentTrainingsJson;
    if (!recentTrainings.isEmpty())
    {
        recentTrainingsJson = QJsonDocument(recentTrainings).toJson(QJsonDocument::Compact);
    }

    templateContent = parseTemplate(templateContent, recentTrainings.isEmpty(), recentTrainingsJson);

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(templateContent);

    emit gptPromptGenerated();
}

void RoutineViewModel::importWorkoutsFromClipboard()
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

WorkoutModel *RoutineViewModel::nextWorkout() const
{
    return next(lastWorkoutName());
}

WorkoutModel *RoutineViewModel::next(const QString &name) const
{
    if (m_workouts.isEmpty())
        return nullptr;

    for (int i = 0; i < m_workouts.size(); ++i)
    {
        WorkoutModel *workout = m_workouts[i];
        if (workout && workout->name() == name)
        {
            int nextIndex = (i + 1) % m_workouts.size();
            return m_workouts[nextIndex];
        }
    }

    return m_workouts.first();
}

bool RoutineViewModel::validateWorkoutsJson(const QString &jsonData, QString &errorMessage)
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
                    errorMessage = QString("Workout %1, exercise %2: missing or invalid 'name'").arg(index).arg(j);
                    return false;
                }
            }
        }

        index++;
    }

    errorMessage.clear();
    return true;
}
