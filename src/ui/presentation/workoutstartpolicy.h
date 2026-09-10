#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class WorkoutModel;

class WorkoutStartPolicy : public QObject
{
    Q_OBJECT

public:
    static constexpr auto actionKey = "action";
    static constexpr auto labelKey = "label";
    static constexpr auto messageKey = "message";
    static constexpr auto confirmationKey = "confirmation";

    static constexpr auto startAction = "start";
    static constexpr auto replaceAction = "replace";
    static constexpr auto resumeAction = "resume";
    static constexpr auto blockedAction = "blocked";
    static constexpr auto missingAction = "missing";

    explicit WorkoutStartPolicy(QObject* parent = nullptr);

    Q_INVOKABLE QVariantMap decide(WorkoutModel* selected, WorkoutModel* active) const;
};
