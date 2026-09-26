#include "domain/workout/generatedworkoutname.h"
#include <QDate>
#include <QFile>
#include <QFontMetricsF>
#include <QRegularExpression>
#include <gtest/gtest.h>

namespace
{

constexpr int narrowest_screen_width = 360;

QString themeSource()
{
    QFile file(QStringLiteral(QML_SOURCE_DIR "/Theme.qml"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();

    return QString::fromUtf8(file.readAll());
}

QString themeGroup(const QString& group)
{
    const QString source = themeSource();
    const int start = source.indexOf(QStringLiteral("%1: QtObject {").arg(group));
    if (start < 0)
        return QString();

    const int end = source.indexOf(QStringLiteral("\n    }"), start);
    return end < 0 ? source.mid(start) : source.mid(start, end - start);
}

int themeNumber(const QString& group, const QString& property)
{
    const QRegularExpression pattern(QStringLiteral("%1:\\s*(\\d+)").arg(property));
    const QRegularExpressionMatch match = pattern.match(themeGroup(group));
    return match.hasMatch() ? match.captured(1).toInt() : -1;
}

int drumTextWidth(const QString& gapProperty)
{
    const int screenPadding = themeNumber(QStringLiteral("padding"), QStringLiteral("screen"));
    const int rowWidth = narrowest_screen_width - 2 * screenPadding;

    return rowWidth - 2 * themeNumber(QStringLiteral("drum"), QStringLiteral("padding"))
        - themeNumber(QStringLiteral("drum"), QStringLiteral("dateWidth"))
        - 2 * themeNumber(QStringLiteral("drum"), gapProperty);
}

QFontMetricsF drumMetrics(int pixelSize, bool bold)
{
    QFont font;
    font.setPixelSize(pixelSize);
    font.setBold(bold);
    return QFontMetricsF(font);
}

QString widestGeneratedSessionName(const QFontMetricsF& metrics)
{
    const QDate day(2026, 9, 10);

    QString widest = generatedPlanName();
    for (int minutes = 0; minutes < 24 * 60; ++minutes)
    {
        const QString name
            = generatedSessionName(QDateTime(day, QTime(0, 0).addSecs(minutes * 60)));
        if (metrics.horizontalAdvance(name) > metrics.horizontalAdvance(widest))
            widest = name;
    }

    return widest;
}

void expectFits(const QFontMetricsF& metrics, const QString& text, int available,
                const QString& typeface)
{
    EXPECT_LE(metrics.horizontalAdvance(text), available)
        << "\"" << text.toStdString() << "\" in " << typeface.toStdString() << " needs "
        << metrics.horizontalAdvance(text) << " px of " << available;
}

}

TEST(DrumFitTest, ThemeCarriesEverySizeTheDrumRowIsMeasuredAgainst)
{
    EXPECT_GT(themeNumber(QStringLiteral("padding"), QStringLiteral("screen")), 0);
    EXPECT_GT(themeNumber(QStringLiteral("drum"), QStringLiteral("padding")), 0);
    EXPECT_GT(themeNumber(QStringLiteral("drum"), QStringLiteral("dateWidth")), 0);
    EXPECT_GT(themeNumber(QStringLiteral("drum"), QStringLiteral("gap")), 0);
    EXPECT_GT(themeNumber(QStringLiteral("drum"), QStringLiteral("gapLarge")), 0);
    EXPECT_GT(themeNumber(QStringLiteral("drum"), QStringLiteral("nameSize")), 0);
    EXPECT_GT(themeNumber(QStringLiteral("drum"), QStringLiteral("nameSizeLarge")), 0);
    EXPECT_GT(themeNumber(QStringLiteral("drum"), QStringLiteral("labelSizeLarge")), 0);
}

TEST(DrumFitTest, TheNameOfAFreeSessionFitsTheSelectedRowOnANarrowScreen)
{
    const int available = drumTextWidth(QStringLiteral("gapLarge"));
    ASSERT_GT(available, 0);

    const QFontMetricsF metrics
        = drumMetrics(themeNumber(QStringLiteral("drum"), QStringLiteral("nameSizeLarge")), true);

    expectFits(metrics, widestGeneratedSessionName(metrics), available,
               QStringLiteral("the selected name font"));
}

TEST(DrumFitTest, TheNameOfAFreeSessionFitsARestingRowOnANarrowScreen)
{
    const int available = drumTextWidth(QStringLiteral("gap"));
    ASSERT_GT(available, 0);

    const QFontMetricsF metrics
        = drumMetrics(themeNumber(QStringLiteral("drum"), QStringLiteral("nameSize")), true);

    expectFits(metrics, widestGeneratedSessionName(metrics), available,
               QStringLiteral("the resting name font"));
}

TEST(DrumFitTest, TheOfferToStartWithoutAPlanFitsBothRowFonts)
{
    const QString offer = QStringLiteral("Start from scratch");

    expectFits(
        drumMetrics(themeNumber(QStringLiteral("drum"), QStringLiteral("nameSizeLarge")), true),
        offer, drumTextWidth(QStringLiteral("gapLarge")),
        QStringLiteral("the selected name font"));

    expectFits(drumMetrics(themeNumber(QStringLiteral("drum"), QStringLiteral("nameSize")), true),
               offer, drumTextWidth(QStringLiteral("gap")),
               QStringLiteral("the resting name font"));
}

TEST(DrumFitTest, TheLabelOfTheFreeSessionRowFitsTheSelectedRowOnANarrowScreen)
{
    const int available = drumTextWidth(QStringLiteral("gapLarge"));
    ASSERT_GT(available, 0);

    QFont font;
    font.setPixelSize(themeNumber(QStringLiteral("drum"), QStringLiteral("labelSizeLarge")));
    font.setLetterSpacing(QFont::AbsoluteSpacing, 1.2);

    expectFits(QFontMetricsF(font), QStringLiteral("WITHOUT A PLAN"), available,
               QStringLiteral("the row label font"));
}
