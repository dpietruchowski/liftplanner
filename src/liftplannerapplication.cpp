#include "liftplannerapplication.h"

#include "application/exercisecatalog/exercisecatalogservice.h"
#include "application/userprofile/userprofileservice.h"
#include "application/workout/workoutservice.h"
#include "application/workout/workouttemplateservice.h"
#include "async/backendworker.h"
#include "infrastructure/appdbstorage.h"
#include "infrastructure/exercisecatalog/catalogdefinitionlookup.h"
#include "infrastructure/exercisecatalog/exercisedefinitionrepositorydb.h"
#include "infrastructure/userprofile/userprofilerepositorydb.h"
#include "infrastructure/workout/workoutrepositorydb.h"
#include "infrastructure/workout/workoutrowrepositorydb.h"
#include "infrastructure/workout/workouttemplaterepositorydb.h"
#include "infrastructure/workout/workouttemplaterowrepositorydb.h"
#include "qmlutils/coloredsvgprovider.h"
#include "qmlutils/qmlregistrator.h"
#include "ui/presentation/appinfo.h"
#include "ui/presentation/clipboardhelper.h"
#include "ui/presentation/notificationtypes.h"
#include "ui/presentation/plural.h"
#include "ui/presentation/workoutstartpolicy.h"
#include "ui/viewmodels/activeworkoutviewmodel.h"
#include "ui/viewmodels/exercisecatalogviewmodel.h"
#include "ui/viewmodels/plannedworkoutviewmodel.h"
#include "ui/viewmodels/userprofileviewmodel.h"
#include "ui/viewmodels/workouteditorviewmodel.h"
#include "ui/viewmodels/workouthistoryviewmodel.h"
#include "ui/viewmodels/workouttemplateviewmodel.h"
#include <QDebug>
#include <QMetaObject>

LiftPlannerApplication::LiftPlannerApplication(const QString& dbPath)
{
    m_worker = std::make_unique<BackendWorker>();
    m_storage = std::make_unique<AppDbStorage>(dbPath);
}

LiftPlannerApplication::~LiftPlannerApplication()
{
    m_worker->drain();

    m_workoutStartPolicy.reset();
    m_appInfo.reset();
    m_clipboardHelper.reset();
    m_workoutTemplateViewModel.reset();
    m_workoutEditorViewModel.reset();
    m_exerciseCatalogViewModel.reset();
    m_userProfileViewModel.reset();
    m_plannedWorkoutViewModel.reset();
    m_workoutHistoryViewModel.reset();
    m_activeWorkoutViewModel.reset();

    QMetaObject::invokeMethod(
        m_worker.get(),
        [this]()
        {
            m_workoutTemplateService.reset();
            m_definitionLookup.reset();
            m_exerciseCatalogService.reset();
            m_userProfileService.reset();
            m_workoutService.reset();
            m_storage.reset();
        },
        Qt::BlockingQueuedConnection);
}

bool LiftPlannerApplication::initialize()
{
    if (!createServices())
    {
        qCritical() << "Failed to open database";
        return false;
    }

    createViewModels();
    return true;
}

bool LiftPlannerApplication::createServices()
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
            m_workoutService = std::make_unique<WorkoutService>(
                m_storage->workoutRepo(), m_storage->workoutRowRepo(), m_worker.get());
            m_userProfileService = std::make_unique<UserProfileService>(
                m_storage->userProfileRepo(), m_worker.get());
            m_exerciseCatalogService = std::make_unique<ExerciseCatalogService>(
                m_storage->exerciseDefinitionRepo(), m_worker.get());
            m_definitionLookup
                = std::make_unique<CatalogDefinitionLookup>(m_storage->exerciseDefinitionRepo());
            m_workoutTemplateService = std::make_unique<WorkoutTemplateService>(
                m_storage->workoutTemplateRepo(), m_storage->workoutTemplateRowRepo(),
                *m_definitionLookup, m_worker.get());
        },
        Qt::BlockingQueuedConnection);

    return opened;
}

void LiftPlannerApplication::createViewModels()
{
    m_activeWorkoutViewModel = std::make_unique<ActiveWorkoutViewModel>(m_workoutService.get());
    m_workoutHistoryViewModel = std::make_unique<WorkoutHistoryViewModel>(
        m_workoutService.get(), m_activeWorkoutViewModel.get());
    m_plannedWorkoutViewModel = std::make_unique<PlannedWorkoutViewModel>(
        m_workoutService.get(), m_userProfileService.get(), m_activeWorkoutViewModel.get());
    m_userProfileViewModel = std::make_unique<UserProfileViewModel>(m_userProfileService.get());
    m_exerciseCatalogViewModel
        = std::make_unique<ExerciseCatalogViewModel>(m_exerciseCatalogService.get());
    m_workoutEditorViewModel = std::make_unique<WorkoutEditorViewModel>(
        m_workoutService.get(), m_workoutTemplateService.get());
    m_workoutTemplateViewModel
        = std::make_unique<WorkoutTemplateViewModel>(m_workoutTemplateService.get());
    m_clipboardHelper = std::make_unique<ClipboardHelper>();
    m_appInfo = std::make_unique<AppInfo>();
    m_pluralText = std::make_unique<PluralText>();
    m_workoutStartPolicy = std::make_unique<WorkoutStartPolicy>();
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
    registrator.registerSingletonInstance("WorkoutTemplateViewModel",
                                          m_workoutTemplateViewModel.get());
    registrator.registerSingletonInstance("ClipboardHelper", m_clipboardHelper.get());
    registrator.registerSingletonInstance("AppInfo", m_appInfo.get());
    registrator.registerSingletonInstance("Plural", m_pluralText.get());
    registrator.registerSingletonInstance("WorkoutStartPolicy", m_workoutStartPolicy.get());

    registrator.registerSingletonType("Themed.Components", "Theme.qml", "Theme");
}
