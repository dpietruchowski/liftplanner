#include "liftplannerapplication.h"
#include "qmlutils/qmlregistrator.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

#ifdef LIBS_AUTOMATION
#include "automation/uiautomationserver.h"
#endif

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    app.setApplicationName(QStringLiteral(APP_NAME));
    app.setApplicationVersion(QStringLiteral(APP_VERSION));

    // Use the same controls style on every platform (Android defaults to
    // Material, desktop to Basic) so the UI renders identically everywhere.
    QQuickStyle::setStyle("Basic");

    LiftPlannerApplication liftApp("liftplanner.db");
    if (!liftApp.initialize())
        return -1;

    QQmlApplicationEngine engine;

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

#ifdef QML_LIVE_ENABLED
    const QString uiRootDir = QStringLiteral(APP_QML_SOURCE_DIR);
#else
    const QString uiRootDir = QStringLiteral("qrc:/" APP_QML_URI);
#endif

    QmlRegistrator registrator(engine, uiRootDir, APP_QML_URI);
    liftApp.registerQmlTypes(registrator);

#ifdef QML_LIVE_ENABLED
    registrator.setupLiveReload();
#endif

    engine.load(registrator.getMainQmlUrl());

#ifdef LIBS_AUTOMATION
    if (!engine.rootObjects().isEmpty())
    {
        const QByteArray automationPort = qgetenv("APP_AUTOMATION_PORT");
        const quint16 port = automationPort.isEmpty() ? 49200 : automationPort.toUShort();
        new UiAutomationServer(&engine, port, &app);
    }
#endif

    return app.exec();
}
