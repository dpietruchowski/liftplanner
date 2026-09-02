#include "workouttemplateviewmodel.h"

#include "modules/workout/application/workouttemplateservice.h"

WorkoutTemplateViewModel::WorkoutTemplateViewModel(WorkoutTemplateService* service, QObject* parent)
    : QObject(parent)
    , m_service(service)
{
}

WorkoutTemplateViewModel::~WorkoutTemplateViewModel()
{
    qDeleteAll(m_templates);
    m_templates.clear();
}

QList<WorkoutTemplateModel*> WorkoutTemplateViewModel::templates() const { return m_templates; }

int WorkoutTemplateViewModel::count() const { return static_cast<int>(m_templates.size()); }

bool WorkoutTemplateViewModel::isEmpty() const { return m_templates.isEmpty(); }

bool WorkoutTemplateViewModel::isLoading() const { return m_loading; }

QString WorkoutTemplateViewModel::searchText() const { return m_searchText; }

void WorkoutTemplateViewModel::setSearchText(const QString& value)
{
    const QString trimmed = value.trimmed();
    if (m_searchText == trimmed)
        return;

    m_searchText = trimmed;
    emit searchTextChanged();
    load();
}

void WorkoutTemplateViewModel::load()
{
    if (!m_service)
        return;

    setLoading(true);

    m_service->searchSummaries(m_searchText)
        .onError(this,
                 [this](const QString& error)
                 {
                     setLoading(false);
                     emit errorOccurred(error);
                 })
        .then(this,
              [this](std::vector<WorkoutTemplateSummary> summaries)
              {
                  qDeleteAll(m_templates);
                  m_templates.clear();
                  for (const auto& summary : summaries)
                      m_templates.append(new WorkoutTemplateModel(summary, this));

                  setLoading(false);
                  emit templatesChanged();
              });
}

void WorkoutTemplateViewModel::duplicate(int templateId)
{
    if (!m_service || findById(templateId) == nullptr)
        return;

    m_service->duplicate(templateId, QString())
        .onError(this, [this](const QString& error) { emit errorOccurred(error); })
        .then(this,
              [this](int newId)
              {
                  emit duplicated(newId);
                  load();
              });
}

void WorkoutTemplateViewModel::remove(int templateId)
{
    if (!m_service || findById(templateId) == nullptr)
        return;

    m_service->remove(templateId)
        .onError(this, [this](const QString& error) { emit errorOccurred(error); })
        .then(this,
              [this, templateId](bool)
              {
                  emit removed(templateId);
                  load();
              });
}

WorkoutTemplateModel* WorkoutTemplateViewModel::findById(int templateId) const
{
    for (auto* model : m_templates)
    {
        if (model->templateId() == templateId)
            return model;
    }
    return nullptr;
}

void WorkoutTemplateViewModel::setLoading(bool value)
{
    if (m_loading == value)
        return;

    m_loading = value;
    emit loadingChanged();
}
