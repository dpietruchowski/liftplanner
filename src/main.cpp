#include "liftplannerapplication.h"
#include "utils/qmlregistrator.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

#ifdef QT_DEBUG
#include "automation/uiautomationserver.h"
#endif

#ifndef APP_VERSION
#define APP_VERSION "dev"
#endif

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    app.setApplicationName("LiftPlanner");
    app.setApplicationVersion(QStringLiteral(APP_VERSION));

    // Use the same controls style on every platform (Android defaults to
    // Material, desktop to Basic) so the UI renders identically everywhere.
    QQuickStyle::setStyle("Basic");

    LiftPlannerApplication liftApp("liftplanner.db");
    liftApp.initialize();

    QQmlApplicationEngine engine;

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

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

#ifdef QT_DEBUG
    const QByteArray automationPort = qgetenv("LIFTPLANNER_AUTOMATION_PORT");
    if (!automationPort.isEmpty() && !engine.rootObjects().isEmpty())
        new UiAutomationServer(&engine, automationPort.toUShort(), &app);
#endif

    return app.exec();
}
