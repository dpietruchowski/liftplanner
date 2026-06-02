#include <QCoreApplication>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("LiftPlannerTest");
    app.setApplicationName("IntegrationTests");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
