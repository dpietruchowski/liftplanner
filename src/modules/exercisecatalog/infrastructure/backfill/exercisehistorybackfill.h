#pragma once

#include <QSqlDatabase>

class ExerciseDefinitionRepository;

struct HistoryBackfillResult
{
    int examined { 0 };
    int linked { 0 };
    int unmatched { 0 };
};

class ExerciseHistoryBackfill
{
public:
    static HistoryBackfillResult apply(QSqlDatabase& database,
                                       const ExerciseDefinitionRepository& catalog);
};
