#pragma once

#include <QVariantMap>

struct MuscleInvolvement;

class MuscleInvolvementSerializer
{
public:
    static constexpr const char* table = "exercise_definition_muscles";
    static constexpr const char* id_key = "id";
    static constexpr const char* definition_id_key = "definition_id";
    static constexpr const char* muscle_key = "muscle";
    static constexpr const char* role_key = "role";
    static constexpr const char* position_key = "position";

    static MuscleInvolvement fromVariant(const QVariantMap& data);
    static QVariantMap toVariant(const MuscleInvolvement& involvement, int definitionId,
                                 int position);
};
