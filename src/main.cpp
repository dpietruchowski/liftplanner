#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include "services/activeworkoutservice.h"
#include "services/routineservice.h"
#include "services/workouthistoryservice.h"
#include "storage/appdbstorage.h"
#include "utils/coloredsvgprovider.h"

#ifdef QML_LIVE_ENABLED
#include "lib/filewatcher.h"
#endif

#include <QDebug>
#include <QDirIterator>
#include <QResource>

void listResources(const QString &prefix)
{
    QDirIterator it(prefix, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        qDebug() << it.next();
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    AppDbStorage storage("liftplanner.db");
    RoutineService routineService(&storage);
    ActiveWorkoutService activeWorkoutService;
    WorkoutHistoryService workoutHistoryService(&storage);

    QQmlApplicationEngine engine;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []()
        { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    qmlRegisterType<ColoredSvgProvider>("LiftPlanner", 1, 0, "ColoredSvgProvider");

#ifdef QML_LIVE_ENABLED
    qDebug() << "Using qml live";
    const QDir watchDir("/home/damian/dev/lift-planner/src/ui");
    const QUrl mainQmlUrl = QUrl::fromLocalFile(watchDir.filePath("Main.qml"));

    qmlRegisterSingletonType(QUrl::fromLocalFile(watchDir.filePath("Theme.qml")),
                             "LiftPlanner",
                             1,
                             0,
                             "Theme");

    engine.addImportPath(watchDir.absolutePath());

    QQmlContext *ctx = engine.rootContext();
    ctx->setContextProperty("RoutineService", &routineService);
    ctx->setContextProperty("ActiveWorkoutService", &activeWorkoutService);
    ctx->setContextProperty("WorkoutHistoryService", &workoutHistoryService);

    engine.load(mainQmlUrl);

    FileWatcher watcher([&, watchDir]()
                        {
        QObject *root = engine.rootObjects().isEmpty() ? nullptr : engine.rootObjects().first();
        if (!root)
            return;

        QObject *loader = root->findChild<QObject *>("mainLoader");
        if (!loader)
            return;

        QUrl viewUrl = QUrl::fromLocalFile(watchDir.filePath("MainView.qml"));

        engine.clearComponentCache();
        loader->setProperty("source", QUrl());
        loader->setProperty("source", viewUrl); });

    watcher.setDirectory(watchDir.absolutePath());

#else
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

    qmlRegisterSingletonType(QUrl("qrc:/LiftPlanner/ui/Theme.qml"), "LiftPlanner", 1, 0, "Theme");
    const QUrl url(QStringLiteral("qrc:/LiftPlanner/ui/Main.qml"));
    engine.load(url);
#endif

    return app.exec();
}
