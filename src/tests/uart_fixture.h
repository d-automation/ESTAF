#pragma once

#include "../core/declarations.h"
#include "../core/uart.h"
#include "../core/logger.h"
#include "../core/test_loader.h"
#include "../core/test_stats.h"
#include <json_processor.h>

class UARTFixture :
    public ::testing::TestWithParam<QString>
{
protected:
    static UART uart;
    static comSettings_t comPortSettings;
    static path_t path;
    static QString logFile;

    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static int readConfig();
    static int convert_map_file();
    static int extract_struct_from_elf(QString structTypeName);

    void TearDown() override;
    void runCase(const QString& caseFile);

private:
};
