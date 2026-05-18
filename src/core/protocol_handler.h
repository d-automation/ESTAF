#pragma once

#include "declarations.h"

class ProtocolHandler
{
public:
    ProtocolHandler(const QJsonObject& expectations);

    void feed(const QByteArray& data);

    bool isDone(QString &msg) const;

    QStringList messages;
    QVariantList mismatches;

private:
    void checkStart();
    void checkSequence();
    void checkEnd();

private:
    QJsonObject exp;
    QString buffer;

    bool startReceived = false;
    bool endReceived = false;

    int sequenceIndex = 0;

};
