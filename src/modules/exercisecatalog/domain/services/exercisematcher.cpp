#include "exercisematcher.h"
#include "utils/textmatcher.h"
#include <QRegularExpression>

QString ExerciseMatcher::normalize(const QString& text)
{
    static const QRegularExpression separators(QStringLiteral("[^a-z0-9]+"));

    const QString folded = TextMatcher::foldAccents(text).toLower();
    return folded.split(separators, Qt::SkipEmptyParts).join(QLatin1Char(' '));
}

std::vector<ExerciseMatcher::Candidate>
ExerciseMatcher::toCandidates(const std::vector<ExerciseDefinition>& definitions)
{
    std::vector<Candidate> candidates;
    candidates.reserve(definitions.size());

    for (const auto& definition : definitions)
        candidates.push_back({ definition.id(), definition.name(), definition.aliases() });

    return candidates;
}

std::optional<int> ExerciseMatcher::singleMatch(const std::vector<int>& matches)
{
    if (matches.size() == 1)
        return matches.front();
    return std::nullopt;
}

std::vector<int> ExerciseMatcher::matchExactName(const QString& name,
                                                 const std::vector<Candidate>& candidates)
{
    std::vector<int> matches;
    for (const auto& candidate : candidates)
    {
        if (TextMatcher::compare(name, candidate.name, false))
            matches.push_back(candidate.id);
    }
    return matches;
}

std::vector<int> ExerciseMatcher::matchAlias(const QString& name,
                                             const std::vector<Candidate>& candidates)
{
    std::vector<int> matches;
    for (const auto& candidate : candidates)
    {
        for (const QString& alias : candidate.aliases)
        {
            if (TextMatcher::compare(name, alias, false))
            {
                matches.push_back(candidate.id);
                break;
            }
        }
    }
    return matches;
}

std::vector<int> ExerciseMatcher::matchNormalized(const QString& name,
                                                  const std::vector<Candidate>& candidates)
{
    const QString needle = normalize(name);

    std::vector<int> matches;
    for (const auto& candidate : candidates)
    {
        bool matched = normalize(candidate.name) == needle;
        for (const QString& alias : candidate.aliases)
        {
            if (matched)
                break;
            matched = normalize(alias) == needle;
        }

        if (matched)
            matches.push_back(candidate.id);
    }
    return matches;
}

std::optional<int> ExerciseMatcher::match(const QString& name,
                                          const std::vector<Candidate>& candidates)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty())
        return std::nullopt;

    if (auto exact = singleMatch(matchExactName(trimmed, candidates)))
        return exact;

    if (auto alias = singleMatch(matchAlias(trimmed, candidates)))
        return alias;

    return singleMatch(matchNormalized(trimmed, candidates));
}
