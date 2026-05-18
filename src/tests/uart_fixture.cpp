#include "uart_fixture.h"

UART UARTFixture::uart;
comSettings_t UARTFixture::comPortSettings;
path_t UARTFixture::path;
QString UARTFixture::logFile;

int UARTFixture::extract_struct_from_elf(QString structTypeName)
{
    QProcess process;
    QString program = path.local_python;

    QStringList arguments;
    arguments << path.elf_parser
              << "-f"
              << path.MK_elf_file
              << "-t"
              << structTypeName
              << "-o"
              << path.iar_json_out;

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
              << "-f"
              << path.MK_map_file
              << "-s"
              << "ENTRY LIST"
              << "-o"
              << path.iar_json_out;

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
    HtmlReport::generate(logFile, htmlFile, curTime1);
}


#include "../core/cli_parser.h"
#include "../core/html_report.h"

extern TestOptions g_options;

TEST_P(UARTFixture, JsonCase)
{
    QString caseFile = GetParam();
    runCase(caseFile);
}

void UARTFixture::runCase(
    const QString& caseFile)
{
    QString msgItog;
    QString msg;
    QString err;
    QString error;


    QFile caseFileJson(caseFile);
//    ASSERT_TRUE(caseFileJson.open(QIODevice::ReadOnly));
    if (!caseFileJson.open(QIODevice::ReadOnly))

    {
        msg = QString("%1, path: %2").arg("json test case not found", caseFile);
        msgItog.append(msg);

        qDebug(logCritical()) << msg << Qt::endl;
        Logger::writeToConsol(QString("\nERROR: %1\n").arg(msg));
    }

    QJsonObject cfg =
        QJsonDocument::fromJson(caseFileJson.readAll()).object();

    QString binaryFile = cfg["binary"].toString();

    QFile cmdFile(binaryFile);
//    ASSERT_TRUE(cmdFile.open(QIODevice::ReadOnly));
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
//    ASSERT_TRUE(uart.send(tx_cmd, err));

    QByteArray rx_data = uart.receive(cfg["timeout_msec"].toInt());

    ProtocolHandler handler(cfg["expectations"].toObject());
    handler.feed(rx_data);

    QString protocolMsg;
    bool passed = handler.isDone(protocolMsg);

    if (!passed)
    { error = QString("Protocol validation failed:\n%1").arg(protocolMsg); }

    Logger::saveTestLog(logFile, handler, cfg, passed, error);

    EXPECT_TRUE(passed);

    if (g_options.delayMs > 0)
    { QThread::msleep(g_options.delayMs); }
}


void UARTFixture::TearDown()
{
//    QString cleanupCase =
//        "configs/test_cases/test_abort.json";

//    QFile f(cleanupCase);

//    if (!f.open(QIODevice::ReadOnly))
//        return;

//    QJsonObject cfg =
//        QJsonDocument::fromJson(
//            f.readAll()
//        ).object();

//    QString binary =
//        cfg["binary"].toString();

//    QFile cmd(binary);

//    if (!cmd.open(QIODevice::ReadOnly))
//        return;

//    QByteArray tx = cmd.readAll();

//    QString err;

//    uart.send(tx, err);

//    uart.receive(
//        cfg["timeout_msec"].toInt()
//    );

//    QThread::msleep(200);
}
