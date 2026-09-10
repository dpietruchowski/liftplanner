#include <QGuiApplication>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");

    QGuiApplication app(argc, argv);
    app.setOrganizationName("LiftPlannerTest");
    app.setApplicationName("UiTests");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
