#include "domain/workout/exercise.h"
#include "domain/workout/set.h"
#include "ui/models/setmodel.h"
#include "ui/presentation/workouttext.h"
#include <QFile>
#include <QFontMetricsF>
#include <QRegularExpression>
#include <gtest/gtest.h>

namespace
{

QString themeSource()
{
    QFile file(QStringLiteral(QML_SOURCE_DIR "/Theme.qml"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();

    return QString::fromUtf8(file.readAll());
}

int themeSetRowNumber(const QString& property)
{
    const QRegularExpression pattern(QStringLiteral("%1:\\s*(\\d+)").arg(property));
    const QRegularExpressionMatch match = pattern.match(themeSource());
    return match.hasMatch() ? match.captured(1).toInt() : -1;
}

QFontMetricsF rowMetrics(int pixelSize, bool bold)
{
    QFont font;
    font.setPixelSize(pixelSize);
    font.setBold(bold);
    return QFontMetricsF(font);
}

QFontMetricsF currentRowMetrics()
{
    return rowMetrics(themeSetRowNumber(QStringLiteral("valueCurrentFontSize")), true);
}

QFontMetricsF restingRowMetrics()
{
    return rowMetrics(themeSetRowNumber(QStringLiteral("valueFontSize")), false);
}

Set weightedSet(int repetitions, double weight, LoadType loadType)
{
    Set set(repetitions, weight);
    set.setLoadType(loadType);
    set.setCompleted(true);
    return set;
}

Set completed(Set set)
{
    set.setCompleted(true);
    return set;
}

std::vector<Set> widestSetsOfEveryFormat()
{
    return { weightedSet(12, 102.5, LoadType::External),
             weightedSet(12, 102.5, LoadType::Added),
             weightedSet(12, 102.5, LoadType::Assisted),
             weightedSet(12, 0.0, LoadType::Band),
             weightedSet(12, 0.0, LoadType::Bodyweight),
             completed(Set::createDuration(5400)),
             completed(Set::createDistance(12400.0, 3900)),
             completed(Set::createDistance(42195.0, 14400)) };
}

QStringList widestHintsOfEveryFormat()
{
    Exercise exercise(QStringLiteral("Widest"), 120);
    for (const Set& set : widestSetsOfEveryFormat())
        exercise.addSet(set);

    return WorkoutText::previousSetHints(exercise, static_cast<int>(exercise.sets().size()));
}

void expectFits(const QFontMetricsF& metrics, const QString& text, int columnWidth,
                const QString& column, const QString& row)
{
    EXPECT_LE(metrics.horizontalAdvance(text), columnWidth)
        << column.toStdString() << " of the " << row.toStdString() << " row cannot show \""
        << text.toStdString() << "\": it needs " << metrics.horizontalAdvance(text)
        << " px of " << columnWidth;
}

}

TEST(SetRowFitTest, ThemeCarriesEverySizeTheSetRowIsMeasuredAgainst)
{
    EXPECT_GT(themeSetRowNumber(QStringLiteral("previousWidth")), 0);
    EXPECT_GT(themeSetRowNumber(QStringLiteral("previousFontSize")), 0);
    EXPECT_GT(themeSetRowNumber(QStringLiteral("primaryWidth")), 0);
    EXPECT_GT(themeSetRowNumber(QStringLiteral("valueMinWidth")), 0);
    EXPECT_GT(themeSetRowNumber(QStringLiteral("valueFontSize")), 0);
    EXPECT_GT(themeSetRowNumber(QStringLiteral("valueCurrentFontSize")), 0);
}

TEST(SetRowFitTest, EveryLoadTypeFitsThePreviousSetColumnWithoutEliding)
{
    const int columnWidth = themeSetRowNumber(QStringLiteral("previousWidth"));
    const int fontSize = themeSetRowNumber(QStringLiteral("previousFontSize"));
    ASSERT_GT(columnWidth, 0);
    ASSERT_GT(fontSize, 0);

    const QFontMetricsF metrics = rowMetrics(fontSize, false);

    for (const QString& hint : widestHintsOfEveryFormat())
        expectFits(metrics, hint, columnWidth, QStringLiteral("the hint"), QStringLiteral("any"));
}

TEST(SetRowFitTest, EveryLoadTypeFitsTheValueColumnsInBothRowStates)
{
    const int primaryWidth = themeSetRowNumber(QStringLiteral("primaryWidth"));
    const int valueWidth = themeSetRowNumber(QStringLiteral("valueMinWidth"));
    ASSERT_GT(primaryWidth, 0);
    ASSERT_GT(valueWidth, 0);

    const QFontMetricsF current = currentRowMetrics();
    const QFontMetricsF resting = restingRowMetrics();

    for (const Set& set : widestSetsOfEveryFormat())
    {
        const SetModel model(set);
        expectFits(current, model.primaryText(), primaryWidth, QStringLiteral("REPS"),
                   QStringLiteral("current"));
        expectFits(resting, model.primaryText(), primaryWidth, QStringLiteral("REPS"),
                   QStringLiteral("resting"));
        expectFits(current, model.secondaryText(), valueWidth, QStringLiteral("KG"),
                   QStringLiteral("current"));
        expectFits(resting, model.secondaryText(), valueWidth, QStringLiteral("KG"),
                   QStringLiteral("resting"));
    }
}
