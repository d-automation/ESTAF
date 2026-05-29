#ifndef LOGGER_H
#define LOGGER_H

#include "protocol_handler.h"

class ProtocolHandler;
class QJsonObject;

class Logger
{
public:

    static void saveTestLog(
            const QString& filename,
            const ProtocolHandler& handler,
            const QJsonObject& cfg,
            bool passed,
            const QString& error = "");


    static void saveTestLog(const QString& filename, const QByteArray &rx, const QJsonObject& cfg,
                            bool passed, QString &error, QString mismatches);

    static void appendSummary(
            const QString& filename,
            int total,
            int passed,
            int failed);

    static void setupLog();
    static void writeStandLog(QtMsgType type, const QMessageLogContext &context,
                              const QString &msg);

    static void writeToConsol(QString msg);

private:

    static void write(
            const QString& file,
            const QString& text);
};

#endif // LOGGER_H
