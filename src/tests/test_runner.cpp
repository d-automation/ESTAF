#include "uart_fixture.h"
#include "../core/cli_parser.h"

extern TestOptions g_options;

std::vector<QString> getJsonTests()
{
    QStringList list;
    std::vector<QString> result;

    try
    {
        list = TestLoader::loadCases(
                g_options.group,
                g_options.caseFilter,
                g_options.repeat);
    }
    catch (ErrOpenFile &errOpen)
    {
        qDebug(logCritical()) << QString("%1, path: %2")
               .arg(errOpen.getMessage(),
                    errOpen.getFilePath());

        Logger::writeToConsol(QString("\nERROR: %1, path: %2\n").arg(
                                  errOpen.getMessage(),errOpen.getFilePath()));
        return result;
    }


    for (const auto& s : list)
    {
        result.push_back(s);
    }

    return result;
}


INSTANTIATE_TEST_SUITE_P(
    JsonTestSuite,
    UARTFixture,
    ::testing::ValuesIn(
        getJsonTests()
    ),
    [](const testing::TestParamInfo<QString>& info)
    {
        QFileInfo fi(info.param);

        return fi.baseName()
            .toStdString();
    }
);



//TEST_F(UARTFixture, SmokeTests)
//{
//    TestStats stats;
//    QString msgItog;
//    QString msg;
//    QStringList tests;

//    bool fail = extract_struct_from_elf("Snapshot_HK_t");
//    if (fail == true) { return; }

//    try
//    {
//        tests = TestLoader::loadCases(g_options.group,
//                                              g_options.caseFilter,
//                                              g_options.repeat);
//    }
//    catch (ErrOpenFile &errOpen)
//    {
//        qDebug(logCritical()) << QString("%1, path: %2")
//               .arg(errOpen.getMessage(),
//                    errOpen.getFilePath());

//        Logger::writeToConsol(QString("\nERROR: %1, path: %2\n").arg(
//                                  errOpen.getMessage(),errOpen.getFilePath()));
//        return;
//    }

//    QString curTime = QDateTime::currentDateTime().toString("dd_MM_yyyy_hh_mm_ss");
//    QString curTime1 = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");

//    QString logFile = QString("%1%2.log").arg(path.test_log_path, curTime);
//    QString htmlFile = path.stand_report_html_path;

//    QFile::remove(logFile);

//    for (const QString &caseFile : qAsConst(tests))
//    {
//        QFile caseFileJson(caseFile);
//        QString error;

//        if (!caseFileJson.open(QIODevice::ReadOnly))

//        {
//            msg = QString("%1, path: %2").arg("json test case not found", caseFile);
//            msgItog.append(msg);

//            qDebug(logCritical()) << msg << Qt::endl;
//            Logger::writeToConsol(QString("\nERROR: %1\n").arg(msg));
//        }

//        QJsonObject cfg =
//                QJsonDocument::fromJson(caseFileJson.readAll()).object();
//        QString binaryFile = cfg["binary"].toString();
//        QFile cmdFile(binaryFile);

//        if (!cmdFile.open(QIODevice::ReadOnly))
//        {
//            msg = QString("%1, path: %2").arg("command binary not found", binaryFile);
//            msgItog.append(msg);

//            qDebug(logCritical()) << msg << Qt::endl;
//            Logger::writeToConsol(QString("\nERROR: %1\n").arg(msg));
//        }

//        QByteArray tx_cmd = cmdFile.readAll();
//        QString err;
//        if (!uart.send(tx_cmd, err))
//        {
//            msg = QString("%1: %2").arg("UART data did not send", err);
//            msgItog.append(msg);

//            qDebug(logCritical()) << msg << Qt::endl;
//            Logger::writeToConsol(QString("\nERROR: %1\n").arg(msg));
//        }
//        QByteArray rx_data = uart.receive(cfg["timeout_msec"].toInt());

//        ProtocolHandler handler(cfg["expectations"].toObject());
//        handler.feed(rx_data);
//        QString msg1;
//        bool passed = handler.isDone(msg1);

//        if (!passed)
//        {
//            msgItog.append(msg1);
//            error = QString("Protocol validation failed: \n %1").arg(msgItog);
//        }

//        Logger::saveTestLog(logFile, handler, cfg, passed, error);
//        stats.total++;
//        if (passed) stats.passed++;
//        else stats.failed++;
//        EXPECT_TRUE(passed);

//        if (g_options.delayMs > 0)
//        {
//            QThread::msleep(g_options.delayMs);
//        }
//    }

//    Logger::appendSummary(logFile, stats.total, stats.passed, stats.failed);
//    HtmlReport::generate(logFile, htmlFile, curTime1);
//}
