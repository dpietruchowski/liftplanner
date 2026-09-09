#pragma once

#include <QString>

namespace AppStoragePaths
{
QString directory();
QString file(const QString& name);
QString database(const QString& name);
}
