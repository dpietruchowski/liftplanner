#pragma once

#include "workout.h"
#include <QDate>
#include <QDateTime>
#include <QString>

QString blankSessionName(const QDate& day);
Workout blankSession(const QDateTime& moment);
