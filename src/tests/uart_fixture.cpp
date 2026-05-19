#include "uart_fixture.h"

UART UARTFixture::uart;
comSettings_t UARTFixture::comPortSettings;
Stats_t UARTFixture::stats;
path_t UARTFixture::path;
UART_cmdReadMemory_t UARTFixture::iData;
char UARTFixture::cmdDataBuf[UART_DATA_MAX_BYTES];
QString UARTFixture::logFile;

int UARTFixture::extract_struct_from_elf(QString structTypeName)
{
    QProcess process;
    QString program = path.local_python;

    QStringList arguments;
    arguments << path.elf_parser
              << "-f" << path.MK_elf_file
              << "-t" << structTypeName
              << "-o" << path.iar_json_out;

    process.start(program, arguments);

    if (!process.waitForFinished(3000)) {
        QString msg = QString("timeout processing scipt %1").arg(path.map_parser);

        qDebug(logCritical()) << msg;
        Logger::writeToConsol(QString("\nERROR: %1").arg(msg));
        return 1;
    }

    int exitCode = process.exitCode();

    if (exitCode != 0) {
        QString msg = QString("scipt %1 return error code %2: %3")
                .arg(path.map_parser,
                     QString(exitCode),
                     QString::fromUtf8(process.readAllStandardError()));

        qDebug(logCritical()) << msg;
        Logger::writeToConsol(QString("\nERROR: %1").arg(msg));
        return 1;
    }

   return 0;
}

int UARTFixture::convert_map_file()
{
    QProcess process;
    QString program = "python";

    QStringList arguments;
    arguments << path.map_parser
              << "-f" << path.MK_map_file
              << "-s" << "ENTRY LIST"
              << "-o" << path.iar_json_out;

    process.start(program, arguments);

    if (!process.waitForFinished(3000)) {
        QString msg = QString("timeout processing scipt %1").arg(path.map_parser);

        qDebug(logCritical()) << msg;
        Logger::writeToConsol(QString("\nERROR: %1").arg(msg));
        return 1;
    }

    int exitCode = process.exitCode();

    if (exitCode != 0) {
        QString msg = QString("scipt %1 return error code %2: %3")
                .arg(path.map_parser,
                     QString(exitCode),
                     QString::fromUtf8(process.readAllStandardError()));

        qDebug(logCritical()) << msg;
        Logger::writeToConsol(QString("\nERROR: %1").arg(msg));
        return 1;
    }

   return 0;
}

int UARTFixture::readConfig()
{
    JsonProcessor json;
    QJsonObject jsonObj;
    QString jsonUart = "UART";
    QString jsonPath = "Path";

    try
    {
        json.openJsonFile("../config.json", jsonObj);
        QJsonObject objUart = jsonObj.value(jsonUart).toObject();
        QJsonObject objPath = jsonObj.value(jsonPath).toObject();

        json.jsonSetComPortSettings(jsonUart, objUart, comPortSettings);
        json.jsonGetStrValue(objPath, "MK_elf_file", path.MK_elf_file, jsonPath);
        json.jsonGetStrValue(objPath, "MK_map_file", path.MK_map_file, jsonPath);
        json.jsonGetStrValue(objPath, "test_log_path", path.test_log_path, jsonPath);
        json.jsonGetStrValue(objPath, "stand_report_html_path", path.stand_report_html_path, jsonPath);
        json.jsonGetStrValue(objPath, "map_parser", path.map_parser, jsonPath);
        json.jsonGetStrValue(objPath, "elf_parser", path.elf_parser, jsonPath);
        json.jsonGetStrValue(objPath, "iar_json_out", path.iar_json_out, jsonPath);
        json.jsonGetStrValue(objPath, "local_python", path.local_python, jsonPath);
    }
        catch (ErrOpenFile &errOpen)
    {
        qDebug(logCritical()) << QString("%1, path: %2")
               .arg(errOpen.getMessage(),
                    errOpen.getFilePath());

        Logger::writeToConsol(QString("\nERROR: %1, path: %2\n").arg(
                                  errOpen.getMessage(),errOpen.getFilePath()));
        return 1;
    }
        catch (ErrInJsonSet &jsonSet)
    {
        if (jsonSet.checkErrFromJson())
        {
            QString msg = QString("%1. %2 : %3. file: %4")
                                     .arg(jsonSet.getIntro(),
                                          jsonSet.getErrMsg(),
                                          jsonSet.getErrFromJson(),
                                          jsonSet.getFname());

            qDebug(logCritical()) << msg;
            Logger::writeToConsol(QString("\nERROR: %1").arg(msg));
            return 1;
        }
        else
        {
            QString msg = QString("%1. %2: \nJSON parameter: %3\nJSON object: %4.\nfile: %5")
                                     .arg(jsonSet.getIntro(),
                                          jsonSet.getErrMsg(),
                                          jsonSet.getParam(),
                                          jsonSet.getJsonObj(),
                                          jsonSet.getFname());

            qDebug(logCritical()) << msg;
            Logger::writeToConsol(QString("\nERROR: %1").arg(msg));
            return 1;
        }
    }

    return 0;
}

void UARTFixture::SetUpTestSuite()
{
    Logger::setupLog();
    memset(&stats, 0, sizeof(stats));

    int fail = UARTFixture::readConfig();
    if (fail == true) { return; }
    fail = UARTFixture::convert_map_file();
    if (fail == true) { return; }

    logFile = QString("%1test_cases.log").arg(path.test_log_path);
    QFile::remove(logFile);

    try
    {
        uart.connect(comPortSettings);
    }
    catch (ErrOpenFile &errOpen)
    {
        qDebug(logCritical()) << QString("%1, path: %2")
               .arg(errOpen.getMessage(),
                    errOpen.getFilePath());

        Logger::writeToConsol(QString("\nERROR: %1, path: %2\n").arg(
                                  errOpen.getMessage(),errOpen.getFilePath()));
        return;
    }
}

void UARTFixture::TearDownTestSuite()
{
    uart.close();

    QString curTime1 = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
    QString htmlFile = path.stand_report_html_path;
    Logger::appendSummary(logFile, stats.total, stats.passed, stats.failed);
    HtmlReport::generate(logFile, htmlFile, curTime1);
}


#include "../core/cli_parser.h"
#include "../core/html_report.h"

extern TestOptions g_options;

TEST_P(UARTFixture, JsonCase)
{
    TestCaseParam param = GetParam();
    QString caseFile = param.path;
    runCase(caseFile);
}

void UARTFixture::writeToLog(QString errMsg, bool passed)
{
    if (!passed)
    {
        stats.failed++;
        qDebug(logCritical()) << errMsg << Qt::endl;
        Logger::writeToConsol(QString("\nERROR: %1\n").arg(errMsg));
    }
    else {
        stats.passed++;
    }

    EXPECT_TRUE(passed);
    stats.total++;
}

void UARTFixture::runCase(
    const QString& caseFile)
{
    QString msgItog;
    QString msg;
    QString err;
    QString error;
    bool passed = false;
    QString protocolMsg;
    QString test_case_path = "";

    QFile caseFileJson(caseFile);

    if (!caseFileJson.open(QIODevice::ReadOnly))
    { writeToLog(QString("%1, path: %2").arg("json test case not found", caseFile), false); }

    QJsonObject cfg =
        QJsonDocument::fromJson(caseFileJson.readAll()).object();

    QJsonObject meta = cfg["meta"].toObject();

    if (meta.isEmpty())
    {
        msg = QString("%1, path: %2").arg("incorrect json file: ", caseFile);
        msgItog.append(msg);

        qDebug(logCritical()) << msg << Qt::endl;
        Logger::writeToConsol(QString("\nERROR: %1\n").arg(msg));
    }


    if (meta["input_data_type"].toString() == "binary")
    {
        QJsonObject param = meta["parameters"].toObject();

        bool ok = false;
        iData.version = param["version"].toInt();

        if (param["addr_user_set"].toBool() == true)
        { iData.addr_begin = param["addr"].toString().toUInt(&ok, 16); }

        if (param["size_user_set"].toBool() == true)
        { iData.size_bytes = param["size"].toInt(); }


        if (iData.size_bytes >= UART_DATA_MAX_BYTES - 1)
        {
            msg = QString("%1: %2")
                    .arg("UART data did not send: ",
                         QString("required structure size %1 too big for UART transmission (MAX: %2). See test_case file: %3")
                         .arg(QString(iData.size_bytes), QString(UART_DATA_MAX_BYTES),
                              test_case_path));
            msgItog.append(msg);

            qDebug(logCritical()) << msg << Qt::endl;
            Logger::writeToConsol(QString("\nERROR: %1\n").arg(msg));
            return;
        }
        else
        {
            formCmdDataInBuffer();
            QByteArray data;

            if (param["cmd_from_binary_file"].toBool() == true)
            {
                QString binaryFile = cfg["binary"].toString();
                QFile cmdFile(binaryFile);
                if (!cmdFile.open(QIODevice::ReadOnly))
                {
                    msg = QString("%1, path: %2").arg("command binary not found", binaryFile);
                    msgItog.append(msg);

                    qDebug(logCritical()) << msg << Qt::endl;
                    Logger::writeToConsol(QString("\nERROR: %1\n").arg(msg));
                }

                data = cmdFile.readAll();
            }
            else
            {
                QByteArray data1(QByteArray::fromRawData(cmdDataBuf, UART_DATA_MAX_BYTES));
                data = data1;
            }

            if (!uart.send(data, err))
            {
                msg = QString("%1: %2").arg("UART data did not send", err);
                msgItog.append(msg);

                qDebug(logCritical()) << msg << Qt::endl;
                Logger::writeToConsol(QString("\nERROR: %1\n").arg(msg));
            }


        }

        QByteArray rx_data = uart.receive(cfg["timeout_msec"].toInt());

        Snapshot_HK_t snapshot;
        memcpy(&snapshot, rx_data.constData(), sizeof(snapshot));

        EXPECT_EQ(snapshot.version, 1);
        EXPECT_EQ(snapshot.size, 222);
        EXPECT_EQ(snapshot.system_error, 0);
        EXPECT_EQ(snapshot.guard.all, 0);

        EXPECT_EQ(snapshot.uart.errorLastOperation, 0);

        QVector<int> excludedIdx;
        excludedIdx.push_back(0);

        for (int i = 0; i < 20; i++)
        {
            bool excludeFlag = false;

            for (int j = 0; j < excludedIdx.size(); j++)
            {
                if (i == excludedIdx[j])
                { excludeFlag = true; break; }

            }

            if (excludeFlag == false)
            { EXPECT_EQ(snapshot.uart.error[i], 0); }
        }

        passed = true;
    }
    else if (meta["input_data_type"].toString() == "text")
    {
        QString binaryFile = cfg["binary"].toString();
        QFile cmdFile(binaryFile);
        if (!cmdFile.open(QIODevice::ReadOnly))
        {
            msg = QString("%1, path: %2").arg("command binary not found", binaryFile);
            msgItog.append(msg);

            qDebug(logCritical()) << msg << Qt::endl;
            Logger::writeToConsol(QString("\nERROR: %1\n").arg(msg));
        }

        QByteArray tx_cmd = cmdFile.readAll();
        if (!uart.send(tx_cmd, err))
        {
            msg = QString("%1: %2").arg("UART data did not send", err);
            msgItog.append(msg);

            qDebug(logCritical()) << msg << Qt::endl;
            Logger::writeToConsol(QString("\nERROR: %1\n").arg(msg));
        }

        QByteArray rx_data = uart.receive(cfg["timeout_msec"].toInt());

        ProtocolHandler handler(cfg["expectations"].toObject());
        handler.feed(rx_data);

        if (!passed)
        { error = QString("Protocol validation failed:\n%1").arg(protocolMsg); }

        passed = handler.isDone(protocolMsg);
        Logger::saveTestLog(logFile, handler, cfg, passed, error);
    }


    stats.total++;
    if (passed) stats.passed++;
    else stats.failed++;
    EXPECT_TRUE(passed);

    if (g_options.delayMs > 0)
    { QThread::msleep(g_options.delayMs); }
}

void UARTFixture::serializeCmdDataBuf(void *dataStruct, int countBytes)
{
    UART_cmdWord_u cmdWord;

    cmdWord.all = 0;
    cmdWord.bit.cmdCode = UART_cmdReadMemory;
    cmdWord.bit.countBytes = countBytes;

    int cntBytesAll = CMD_WORD_SIZE + cmdWord.bit.countBytes;

    memcpy(cmdDataBuf, &cmdWord.all, CMD_WORD_SIZE);
    memcpy(cmdDataBuf + CMD_WORD_SIZE, dataStruct,
           cmdWord.bit.countBytes - 1);

    quint8 crc = crc8(cmdDataBuf, cntBytesAll - 1);
    cmdDataBuf[cntBytesAll - 1] = crc;
}


void UARTFixture::formCmdDataInBuffer()
{
    int countBytesCmdData = sizeof(UART_cmdReadMemory_t) + 1;
    serializeCmdDataBuf(&iData, countBytesCmdData);
}

void UARTFixture::TearDown()
{
//    QThread::msleep(200);
//    QString msgItog;
//    QString msg;
//    QString err;
//    QString test_case_path = "";

//    iData.version = 0x1;
//    iData.addr_begin = 0x08125600;
//    iData.size_bytes = 222;

//    if (iData.size_bytes >= UART_DATA_MAX_BYTES - 1)
//    {
//        msg = QString("%1: %2")
//                .arg("UART data did not send: ",
//                     QString("required structure size %1 too big for UART transmission (MAX: %2). See test_case file: %3")
//                     .arg(QString(iData.size_bytes), QString(UART_DATA_MAX_BYTES),
//                          test_case_path));
//        msgItog.append(msg);

//        qDebug(logCritical()) << msg << Qt::endl;
//        Logger::writeToConsol(QString("\nERROR: %1\n").arg(msg));
//        return;
//    }
//    else
//    {
//        formCmdDataInBuffer();

//        QByteArray data(QByteArray::fromRawData(cmdDataBuf, UART_DATA_MAX_BYTES));


//        QString pathData = "data.bin";
//        QFile file(pathData);

//        if (!file.open(QIODevice::WriteOnly))
//        { printf("\t\n ERROR open file!"); }
//        else
//        {
//            QDataStream out(&file);
//            out.writeRawData(data, UART_DATA_MAX_BYTES);
//            file.close();
//        }

//        uart.send(data, err);

//        if (!uart.send(data, err))
//        {
//            msg = QString("%1: %2").arg("UART data did not send", err);
//            msgItog.append(msg);

//            qDebug(logCritical()) << msg << Qt::endl;
//            Logger::writeToConsol(QString("\nERROR: %1\n").arg(msg));
//        }
////        QByteArray rx_data = uart.receive(cfg["timeout_msec"].toInt());
//        QByteArray rx_data = uart.receive(1000);
//        printf(rx_data);
//    }

//    QThread::msleep(200);
}
