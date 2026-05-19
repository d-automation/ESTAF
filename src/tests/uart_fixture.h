#pragma once

#include "../core/declarations.h"
#include "../core/uart.h"
#include "../core/logger.h"
#include "../core/test_loader.h"
#include "../core/test_stats.h"
#include <json_processor.h>

#define CMD_WORD_SIZE           4
#define UART_DATA_MAX_BYTES		1024

struct TestCaseParam
{
    QString path;
    QString name;
};

struct Stats_t
{
    uint16_t total;
    uint16_t passed;
    uint16_t failed;
};

#pragma pack(push,1)
typedef struct
{
    uint32_t version;
    uint32_t addr_begin;
    uint32_t size_bytes;
    uint32_t reserve[20];
} UART_cmdReadMemory_t;
#pragma pack(pop)

typedef union
{
    quint32 all;
    struct BITS
    {
        unsigned reserve: 		2;
        unsigned beginCmdFlag: 	1;
        unsigned endCmdFlag:	1;
        unsigned countPack:		10;
        unsigned countBytes:	10;
        unsigned cmdCode:		8;
    } bit;
} UART_cmdWord_u;

typedef enum
{
    UART_cmdErase = 1,
    UART_cmdAbort = 2,
    UART_cmdBuildBadBlockMapNAND = 3,
    UART_cmdBadBlockMapOper = 4,
    UART_cmdSetNANDsettings = 5,
    UART_cmdReadPage = 6,
    UART_cmdSetDataInterface = 7,
    UART_cmdRandomDataTest = 19,
    UART_cmdMarchFTEallTarget = 20,
    UART_cmdMarchFTE = 21,
    UART_cmdEnduranceTest = 22,
    UART_cmdStartRetentionData = 23,
    UART_cmdCheckRetentionTest = 24,
    UART_cmdNotAbort = 25,
    UART_cmdFail = 26,
    UART_cmdReadMemory = 27,
    UART_cmdEND	= 255
} UART_cmd_en;


class UARTFixture :
    public ::testing::TestWithParam<TestCaseParam>
{
protected:
    static UART uart;
    static Stats_t stats;
    static comSettings_t comPortSettings;
    static path_t path;
    static QString logFile;
    static UART_cmdReadMemory_t iData;
    static char cmdDataBuf[UART_DATA_MAX_BYTES];

    static void serializeCmdDataBuf(void *dataStruct, int countBytes);
    static void formCmdDataInBuffer();
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static int readConfig();
    static int convert_map_file();
    static int extract_struct_from_elf(QString structTypeName);

    void TearDown() override;
    void runCase(const QString& caseFile);

    static void writeToLog(QString errMsg, bool passed);

private:
};
