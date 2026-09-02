#include "liftplannerapplication.h"

#include "core/storage/appdbstorage.h"
#include "modules/exercisecatalog/application/exercisecatalogservice.h"
#include "modules/exercisecatalog/infrastructure/database/exercisedefinitionrepositorydb.h"
#include "modules/exercisecatalog/infrastructure/lookup/catalogdefinitionlookup.h"
#include "modules/userprofile/application/userprofileservice.h"
#include "modules/userprofile/infrastructure/database/userprofilerepositorydb.h"
#include "modules/workout/application/workoutservice.h"
#include "modules/workout/application/workouttemplateservice.h"
#include "modules/workout/infrastructure/database/workoutrepositorydb.h"
#include "modules/workout/infrastructure/database/workouttemplaterepositorydb.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/exercisecatalogviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"
#include "ui/viewmodels/userprofileviewmodel.h"
#include "ui/viewmodels/workouteditorviewmodel.h"
#include "ui/viewmodels/workouthistoryviewmodel.h"
#include "utils/backendworker.h"
#include "utils/clipboardhelper.h"
#include "utils/coloredsvgprovider.h"
#include "utils/notificationtypes.h"
#include "utils/qmlregistrator.h"
#include <QDebug>
#include <QMetaObject>

LiftPlannerApplication::LiftPlannerApplication(const QString& dbPath)
{
    m_worker = std::make_unique<BackendWorker>();
    m_storage = std::make_unique<AppDbStorage>(dbPath);
}

LiftPlannerApplication::~LiftPlannerApplication() = default;

void LiftPlannerApplication::initialize()
{
    bool opened = true;
    QMetaObject::invokeMethod(
        m_worker.get(),
        [this, &opened]()
        {
            if (!m_storage->open())
            {
                opened = false;
                return;
            }
            m_workoutService
                = std::make_unique<WorkoutService>(m_storage->workoutRepo(), m_worker.get());
            m_userProfileService = std::make_unique<UserProfileService>(
                m_storage->userProfileRepo(), m_worker.get());
            m_exerciseCatalogService = std::make_unique<ExerciseCatalogService>(
                m_storage->exerciseDefinitionRepo(), m_worker.get());
            m_definitionLookup
                = std::make_unique<CatalogDefinitionLookup>(m_storage->exerciseDefinitionRepo());
            m_workoutTemplateService = std::make_unique<WorkoutTemplateService>(
                m_storage->workoutTemplateRepo(), *m_definitionLookup, m_worker.get());
        },
        Qt::BlockingQueuedConnection);

    if (!opened)
    {
        qCritical() << "Failed to open database";
        return;
    }

    m_activeWorkoutViewModel = std::make_unique<ActiveWorkoutViewModel>(m_workoutService.get());
    m_workoutHistoryViewModel = std::make_unique<WorkoutHistoryViewModel>(
        m_workoutService.get(), m_activeWorkoutViewModel.get());
    m_plannedWorkoutViewModel = std::make_unique<PlannedWorkoutViewModel>(
        m_workoutService.get(), m_userProfileService.get());
    m_userProfileViewModel = std::make_unique<UserProfileViewModel>(m_userProfileService.get());
    m_exerciseCatalogViewModel
        = std::make_unique<ExerciseCatalogViewModel>(m_exerciseCatalogService.get());
    m_workoutEditorViewModel = std::make_unique<WorkoutEditorViewModel>(
        m_workoutService.get(), m_workoutTemplateService.get());
    m_clipboardHelper = std::make_unique<ClipboardHelper>();
}

void LiftPlannerApplication::registerQmlTypes(QmlRegistrator& registrator)
{
    registrator.registerEnums(Notification::staticMetaObject, "Notification");

    registrator.registerType<ColoredSvgProvider>("Themed.Components", "ColoredSvgProvider");

    registrator.registerSingletonInstance("ActiveWorkoutViewModel", m_activeWorkoutViewModel.get());
    registrator.registerSingletonInstance("WorkoutHistoryViewModel",
                                          m_workoutHistoryViewModel.get());
    registrator.registerSingletonInstance("PlannedWorkoutViewModel",
                                          m_plannedWorkoutViewModel.get());
    registrator.registerSingletonInstance("UserProfileViewModel", m_userProfileViewModel.get());
    registrator.registerSingletonInstance("ExerciseCatalogViewModel",
                                          m_exerciseCatalogViewModel.get());
    registrator.registerSingletonInstance("WorkoutEditorViewModel", m_workoutEditorViewModel.get());
    registrator.registerSingletonInstance("ClipboardHelper", m_clipboardHelper.get());

    registrator.registerSingletonType("Theme.qml", "Theme");
    registrator.registerSingletonType("Themed.Components", "Theme.qml", "Theme");
}
