#pragma once

#include <QString>

enum class PrimaryGoal
{
    GeneralFitness,
    WeightLoss,
    MuscleGain,
    Strength,
    Endurance
};

inline QString primaryGoalToString(PrimaryGoal goal)
{
    switch (goal)
    {
        case PrimaryGoal::GeneralFitness:
            return QStringLiteral("general_fitness");
        case PrimaryGoal::WeightLoss:
            return QStringLiteral("weight_loss");
        case PrimaryGoal::MuscleGain:
            return QStringLiteral("muscle_gain");
        case PrimaryGoal::Strength:
            return QStringLiteral("strength");
        case PrimaryGoal::Endurance:
            return QStringLiteral("endurance");
    }
    return QStringLiteral("general_fitness");
}

inline PrimaryGoal primaryGoalFromString(const QString& str)
{
    if (str == QStringLiteral("weight_loss"))
        return PrimaryGoal::WeightLoss;
    if (str == QStringLiteral("muscle_gain"))
        return PrimaryGoal::MuscleGain;
    if (str == QStringLiteral("strength"))
        return PrimaryGoal::Strength;
    if (str == QStringLiteral("endurance"))
        return PrimaryGoal::Endurance;
    return PrimaryGoal::GeneralFitness;
}
