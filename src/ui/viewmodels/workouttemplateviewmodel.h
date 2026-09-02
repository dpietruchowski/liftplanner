#pragma once

#include "ui/models/workouttemplatemodel.h"
#include <QList>
#include <QObject>
#include <QString>

class WorkoutTemplateService;

class WorkoutTemplateViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<WorkoutTemplateModel*> templates READ templates NOTIFY templatesChanged)
    Q_PROPERTY(int count READ count NOTIFY templatesChanged)
    Q_PROPERTY(bool empty READ isEmpty NOTIFY templatesChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

public:
    explicit WorkoutTemplateViewModel(WorkoutTemplateService* service, QObject* parent = nullptr);
    ~WorkoutTemplateViewModel();

    QList<WorkoutTemplateModel*> templates() const;
    int count() const;
    bool isEmpty() const;
    bool isLoading() const;
    QString searchText() const;

    void setSearchText(const QString& value);

    Q_INVOKABLE void load();
    Q_INVOKABLE void duplicate(int templateId);
    Q_INVOKABLE void remove(int templateId);
    Q_INVOKABLE WorkoutTemplateModel* findById(int templateId) const;

signals:
    void templatesChanged();
    void loadingChanged();
    void searchTextChanged();
    void removed(int templateId);
    void duplicated(int templateId);
    void errorOccurred(const QString& errorMessage);

private:
    void setLoading(bool value);

    WorkoutTemplateService* m_service;
    QList<WorkoutTemplateModel*> m_templates;
    QString m_searchText;
    bool m_loading { false };
};
