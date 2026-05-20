#include "logger.h"

// Умный указатель на файл логирования
QScopedPointer<QFile>   m_logFile;

void Logger::write(const QString& file, const QString& text)
{
    QFile f(file);

    f.open(QIODevice::Append | QIODevice::Text);

    QTextStream out(&f);

    out << text;

    f.close();
}

void Logger::saveTestLog(const QString& filename, const QJsonObject& cfg,
                         bool passed, QString& error, QString mismatches)
{
    QString log;

    log += "\n<<<TEST_START>>>\n";

    log += "TEST: ";
    log += cfg["meta"].toObject()["name"].toString();
    log += "\n";

    log += "TIME: ";
    log += QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
    log += "\n";

    log += passed ? "STATUS: PASS\n" : "STATUS: FAIL\n";

    if (!error.isEmpty())
    {
        log += "\n--- ERROR ---\n";
        log += error;
        log += "\n";
    }

    if (!mismatches.isEmpty())
    {
        log += "\n--- MISMATCHES ---\n";
        log += mismatches;
        log += "\n";
    }

    log += "<<<TEST_END>>>\n";

    write(filename, log);
}


void Logger::saveTestLog(
        const QString& filename,
        const ProtocolHandler& handler,
        const QJsonObject& cfg,
        bool passed,
        const QString& error)
{
    QString log;

    log += "\n<<<TEST_START>>>\n";

    log += "TEST: ";
    log += cfg["meta"].toObject()["name"].toString();
    log += "\n";

    log += "TIME: ";
    log += QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
    log += "\n";

    log += passed ? "STATUS: PASS\n" : "STATUS: FAIL\n";

    if (!error.isEmpty())
    {
        log += "\n--- ERROR ---\n";
        log += error;
        log += "\n";
    }

    if (!handler.mismatches.isEmpty())
    {
        log += "\n--- MISMATCHES ---\n";

        QJsonDocument doc(QJsonArray::fromVariantList(handler.mismatches));

        log += doc.toJson();
        log += "\n";
    }

    log += "\n--- FULL LOG ---\n";

    for (const QString& s : handler.messages)
    {
        log += s;
        log += "\n";
    }

    log += "<<<TEST_END>>>\n";

    write(filename, log);
}

void Logger::appendSummary(const QString& filename, int total,
                           int passed, int failed)
{
    QString s;

    s += "\n";
    s += "############################################################\n";
    s += "SUMMARY\n";
    s += "############################################################\n";

    s += QString("TOTAL: %1\n").arg(total);
    s += QString("PASSED: %1\n").arg(passed);
    s += QString("FAILED: %1\n").arg(failed);

    write(filename, s);
}

void Logger::setupLog()
{
    // Устанавливаем файл логирования

    m_logFile.reset(new QFile("C:/Danila/work/embedded_test_stand/logs/test_stand.log"));
    // Открываем файл логирования
    if (!m_logFile.data()->open(QFile::Append | QFile::Text))
    { printf("\nlog file is not opened!\n"); }
    // Устанавливаем обработчик
    qInstallMessageHandler(writeStandLog);
}

void Logger::writeToConsol(QString msg)
{
    QTextStream out(stdout);
    out.setCodec("IBM 866");
    out << msg << Qt::endl;
}

void Logger::writeStandLog(QtMsgType type,
                           const QMessageLogContext &context,
                           const QString &msg)
{
    // Открываем поток записи в файл
    QTextStream out(m_logFile.data());
    // Записываем дату записи
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
    // По типу определяем, к какому уровню относится сообщение
    switch (type)
    {
    case QtInfoMsg:     out << "INF "; break;
    case QtDebugMsg:    out << "DBG "; break;
    case QtWarningMsg:  out << "WRN "; break;
    case QtCriticalMsg: out << "CRT "; break;
    case QtFatalMsg:    out << "FTL "; break;
    }
    // Записываем в вывод категорию сообщения и само сообщение
    out << context.category << ": " << msg << Qt::endl;
    out.flush();    // Очищаем буферизированные данные

}
