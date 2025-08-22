#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "storage/appdbstorage.h"
#include "services/workoutservice.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    AppDbStorage storage("liftplanner.db");
    WorkoutService workoutService(&storage);

    QQmlApplicationEngine engine;
    
    qmlRegisterSingletonInstance<WorkoutService>("LiftPlanner", 1, 0, "WorkoutService", &workoutService);
    
    const QUrl url(QStringLiteral("qrc:/LiftPlanner/ui/Main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}