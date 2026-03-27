#include "liftplannerapplication.h"

#include "core/storage/appdbstorage.h"
#include "modules/workout/application/workoutservice.h"
#include "modules/workout/infrastructure/database/workoutrepositorydb.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/routineviewmodel.h"
#include "ui/viewmodels/workouthistoryviewmodel.h"
#include "utils/clipboardhelper.h"
#include "utils/coloredsvgprovider.h"
#include "utils/notificationtypes.h"
#include "utils/qmlregistrator.h"

LiftPlannerApplication::LiftPlannerApplication(const QString &dbPath)
{
    m_storage = std::make_unique<AppDbStorage>(dbPath);
}

LiftPlannerApplication::~LiftPlannerApplication() = default;

void LiftPlannerApplication::initialize()
{
    m_workoutService = std::make_unique<WorkoutService>(m_storage->workoutRepo());

    m_routineViewModel = std::make_unique<RoutineViewModel>(m_workoutService.get());
    m_activeWorkoutViewModel = std::make_unique<ActiveWorkoutViewModel>();
    m_workoutHistoryViewModel = std::make_unique<WorkoutHistoryViewModel>(m_workoutService.get());
    m_clipboardHelper = std::make_unique<ClipboardHelper>();
}

void LiftPlannerApplication::registerQmlTypes(QmlRegistrator &registrator)
{
    registrator.registerEnums(Notification::staticMetaObject, "Notification");

    registrator.registerType<ColoredSvgProvider>("Themed.Components", "ColoredSvgProvider");

    registrator.registerSingletonInstance("RoutineService", m_routineViewModel.get());
    registrator.registerSingletonInstance("ActiveWorkoutService", m_activeWorkoutViewModel.get());
    registrator.registerSingletonInstance("WorkoutHistoryService", m_workoutHistoryViewModel.get());
    registrator.registerSingletonInstance("ClipboardHelper", m_clipboardHelper.get());

    registrator.registerSingletonType("Theme.qml", "Theme");
    registrator.registerSingletonType("Themed.Components", "Theme.qml", "Theme");
}
