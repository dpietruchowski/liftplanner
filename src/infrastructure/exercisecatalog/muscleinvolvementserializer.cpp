#include "muscleinvolvementserializer.h"
#include "domain/exercisecatalog/muscleinvolvement.h"

MuscleInvolvement MuscleInvolvementSerializer::fromVariant(const QVariantMap& data)
{
    MuscleInvolvement involvement;
    involvement.muscle = muscleFromString(data.value(muscle_key).toString());
    involvement.role = muscleRoleFromString(data.value(role_key).toString());
    return involvement;
}

QVariantMap MuscleInvolvementSerializer::toVariant(const MuscleInvolvement& involvement,
                                                   int definitionId, int position)
{
    QVariantMap data;
    data.insert(definition_id_key, definitionId);
    data.insert(muscle_key, muscleToString(involvement.muscle));
    data.insert(role_key, muscleRoleToString(involvement.role));
    data.insert(position_key, position);
    return data;
}
