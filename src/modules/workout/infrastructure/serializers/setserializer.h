#pragma once

#include <QVariantMap>

class Set;

class SetSerializer
{
public:
    static constexpr const char *table = "sets";
    static constexpr const char *id_key = "id";
    static constexpr const char *exercise_id_key = "exercise_id";
    static constexpr const char *repetitions_key = "repetitions";
    static constexpr const char *weight_key = "weight";

    static Set fromVariant(const QVariantMap &data);
    static QVariantMap toVariant(const Set &set);
};
