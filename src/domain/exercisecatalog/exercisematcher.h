#pragma once

#include "domain/exercisecatalog/exercisedefinition.h"
#include <QString>
#include <QStringList>
#include <optional>
#include <vector>

class ExerciseMatcher final
{
public:
    struct Candidate
    {
        int id { -1 };
        QString name;
        QStringList aliases;
    };

    static std::optional<int> match(const QString& name, const std::vector<Candidate>& candidates);
    static std::vector<Candidate> toCandidates(const std::vector<ExerciseDefinition>& definitions);
    static QString normalize(const QString& text);

private:
    static std::optional<int> singleMatch(const std::vector<int>& matches);
    static std::vector<int> matchExactName(const QString& name,
                                           const std::vector<Candidate>& candidates);
    static std::vector<int> matchAlias(const QString& name,
                                       const std::vector<Candidate>& candidates);
    static std::vector<int> matchNormalized(const QString& name,
                                            const std::vector<Candidate>& candidates);
};
