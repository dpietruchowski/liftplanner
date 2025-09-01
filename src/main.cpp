#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "services/activeworkoutservice.h"
#include "services/routineservice.h"
#include "services/workouthistoryservice.h"
#include "storage/appdbstorage.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    AppDbStorage storage("liftplanner.db");
    RoutineService routineService(&storage);
    ActiveWorkoutService activeWorkoutService;
    WorkoutHistoryService workoutHistoryService(&storage);

    QQmlApplicationEngine engine;

    qmlRegisterSingletonType(QUrl("qrc:/LiftPlanner/ui/Theme.qml"), "LiftPlanner", 1, 0, "Theme");

    qmlRegisterSingletonInstance<RoutineService>("LiftPlanner",
                                                 1,
                                                 0,
                                                 "RoutineService",
                                                 &routineService);
    qmlRegisterSingletonInstance<ActiveWorkoutService>("LiftPlanner",
                                                       1,
                                                       0,
                                                       "ActiveWorkoutService",
                                                       &activeWorkoutService);
    qmlRegisterSingletonInstance<WorkoutHistoryService>("LiftPlanner",
                                                        1,
                                                        0,
                                                        "WorkoutHistoryService",
                                                        &workoutHistoryService);

    const QUrl url(QStringLiteral("qrc:/LiftPlanner/ui/Main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, []()
                     { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
