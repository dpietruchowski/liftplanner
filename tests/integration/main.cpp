#include <QCoreApplication>
#include <QTemporaryDir>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    QTemporaryDir dataHome;
    if (!dataHome.isValid())
        return 1;
    qputenv("XDG_DATA_HOME", dataHome.path().toUtf8());

    QCoreApplication app(argc, argv);
    app.setOrganizationName("LiftPlannerTest");
    app.setApplicationName("IntegrationTests");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
