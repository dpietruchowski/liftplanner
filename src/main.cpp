#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "services/activeworkoutservice.h"
#include "services/workoutservice.h"
#include "storage/appdbstorage.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    AppDbStorage storage("liftplanner.db");
    WorkoutService workoutService(&storage);
    ActiveWorkoutService activeWorkoutService;

    QQmlApplicationEngine engine;

    qmlRegisterSingletonType(QUrl("qrc:/LiftPlanner/ui/Theme.qml"), "LiftPlanner", 1, 0, "Theme");

    qmlRegisterSingletonInstance<WorkoutService>("LiftPlanner",
                                                 1,
                                                 0,
                                                 "WorkoutService",
                                                 &workoutService);
    qmlRegisterSingletonInstance<ActiveWorkoutService>("LiftPlanner",
                                                       1,
                                                       0,
                                                       "ActiveWorkoutService",
                                                       &activeWorkoutService);

    const QUrl url(QStringLiteral("qrc:/LiftPlanner/ui/Main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
