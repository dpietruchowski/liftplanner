#include "routineservice.h"
#include <QClipboard>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>

namespace {

QString parseTemplate(const QString &templateContent,
                      bool recentTrainingsEmpty,
                      const QString &recentTrainingsJson = QString())
{
    QString result = templateContent;

    QRegularExpression
        ifPattern(R"(\{% if recent_trainings_empty %\}(.*?)\{% else %\}(.*?)\{% endif %\})",
                  QRegularExpression::DotMatchesEverythingOption);

    QRegularExpressionMatch match = ifPattern.match(result);
    if (match.hasMatch()) {
        QString trueBlock = match.captured(1).trimmed();
        QString falseBlock = match.captured(2).trimmed();

        if (recentTrainingsEmpty) {
            result.replace(match.captured(0), trueBlock);
        } else {
            result.replace(match.captured(0), falseBlock);
        }
    }

    if (!recentTrainingsEmpty && !recentTrainingsJson.isEmpty()) {
        result.replace("{{recent_trainings_json}}", recentTrainingsJson);
    } else {
        result.replace("{{recent_trainings_json}}", "");
    }

    return result;
}

QString readTemplateFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open template file:" << filePath;
        return QString();
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    return content;
}

} // namespace

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
    QString templateContent = readTemplateFile(":/LiftPlanner/data/gpt_prompt_template.txt");

    if (templateContent.isEmpty()) {
        qWarning() << "GPT template content is emtpy";
        return;
    }

    QString recentTrainingsJson;
    if (!recentTrainings.isEmpty()) {
        recentTrainingsJson = QJsonDocument(recentTrainings).toJson(QJsonDocument::Compact);
    }

    templateContent = parseTemplate(templateContent, recentTrainings.isEmpty(), recentTrainingsJson);

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(templateContent);

    emit gptPromptGenerated();
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
