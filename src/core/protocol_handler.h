#pragma once

#include "declarations.h"
#include <json_processor.h>

class ProtocolHandler
{
public:
    ProtocolHandler(const QJsonObject& expectations,
                    QString caseFile, QString jsonObjName);
    bool feed(const QByteArray& data, QString& msgError);
    bool isDone(QString &msg) const;

    QStringList messages;
    QVariantList mismatches;

private:
    bool checkStart(QString& msgErr);
    bool checkSequence(QString& msgErr);
    bool checkEnd(QString& msgErr);

private:
    QJsonObject exp;
    QString expStr;
    JsonProcessor json;

    QString buffer;

    bool startReceived = false;
    bool endReceived = false;

    int sequenceIndex = 0;

};
