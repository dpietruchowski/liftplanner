#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "liftplannerapplication.h"
#include "utils/qmlregistrator.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    LiftPlannerApplication liftApp("liftplanner.db");
    liftApp.initialize();

    QQmlApplicationEngine engine;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []()
        { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

#ifdef QML_LIVE_ENABLED
    const QString uiRootDir = "/home/damian/dev/lift-planner/src/ui";
#else
    const QString uiRootDir = "qrc:/LiftPlanner/ui";
#endif

    QmlRegistrator registrator(engine, uiRootDir, "LiftPlanner");
    liftApp.registerQmlTypes(registrator);

#ifdef QML_LIVE_ENABLED
    registrator.setupLiveReload();
#endif

    engine.load(registrator.getMainQmlUrl());

    return app.exec();
}
