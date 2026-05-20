#include <QCoreApplication>

#include "../core/cli_parser.h"
#include <gtest/gtest.h>
TestOptions g_options;

//test_runner.exe --group smoke
//test_runner.exe --group full --repeat 5
//test_runner.exe --group nand_flash --case test_abort
//test_runner.exe --group smoke --delay 1000

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    printf("Embedded test stand v1.2\n");

    g_options = CLIParser::parse(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);

    auto status = RUN_ALL_TESTS();
    return status;
}
