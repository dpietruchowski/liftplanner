#include "blanksession.h"
#include "generatedworkoutname.h"

Workout blankSession(const QDateTime& moment)
{
    Workout session(generatedPlanName(), moment);
    session.setPlannedTime(moment);
    session.setGeneratedName(true);
    return session;
}
