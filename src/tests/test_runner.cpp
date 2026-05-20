#include "uart_fixture.h"
#include "../core/cli_parser.h"

extern TestOptions g_options;

QString prepareTestName(QString name)
{
    // replace invalid symbols into "_"
    name.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");

    // prepend T is digit first
    if (!name.isEmpty() && name[0].isDigit()) { name.prepend("T_"); }

    // protection from empty name:
    if (name.isEmpty()) { name = "unnamed"; }

    return name;
}

std::vector<TestCaseParam> getJsonTests()
{
    QStringList list;
    std::vector<TestCaseParam> result;
    QMap<QString, int> counters;

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

    for (const QString &path : list)
    {
        QFile f(path);
        QString testName = "unknown";

        if (f.open(QIODevice::ReadOnly))
        {
            QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
            testName = obj["meta"].toObject()["name"].toString();
        }

        int index = counters[testName]++;

        TestCaseParam p;
        p.path = path;
        p.name = prepareTestName(QString("%1_%2").arg(testName).arg(index));

        result.push_back(p);
    }

    return result;
}


INSTANTIATE_TEST_SUITE_P(
    JsonTestSuite,
    UARTFixture,
    ::testing::ValuesIn(getJsonTests()),
    [](const testing::TestParamInfo<TestCaseParam>& info)
    {
        return info.param.name.toStdString();
    }
);
