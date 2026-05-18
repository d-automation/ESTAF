#include "protocol_handler.h"


ProtocolHandler::ProtocolHandler(const QJsonObject& expectations)
{
    exp = expectations;
}

void ProtocolHandler::feed(const QByteArray& data)
{
    QString text = QString::fromUtf8(data);

    buffer += text;
    messages.push_back(text);

    QJsonValue startExp = exp["start"];
    QJsonValue endExp = exp["end"];

    if (startExp.isNull())
    { startReceived = true; }

    if (!startReceived)
    { checkStart(); }

    if (startReceived)
    { checkSequence(); }

    if (endExp.isNull())
    { endReceived = true; }

    if (!endReceived)
    { checkEnd(); }
}

void ProtocolHandler::checkStart()
{
    if (!buffer.contains("{"))
        return;

    int start = buffer.indexOf("{");
    int end = buffer.indexOf("}");

    if (end < 0)
        return;

    QString jsonText = buffer.mid(start, end - start + 1);

    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8());

    if (!doc.isObject()) return;

    QJsonObject msg = doc.object();

    QJsonObject expected = exp["start"]
            .toObject()["fields"]
            .toObject();

    bool ok = true;

    for (QString key : expected.keys())
    {
        if (msg[key] != expected[key])
        {
            ok = false;
            break;
        }
    }

    if (ok)
    { startReceived = true; }
    else
    {
        QVariantMap m;

        m["stage"] = "start";
        m["expected"] = expected;
        m["actual"] = msg.toVariantMap();

        mismatches.append(m);
    }

    buffer = buffer.mid(end + 1);
}

void ProtocolHandler::checkSequence()
{
    QJsonArray seq = exp["sequence"].toArray();

    while (sequenceIndex < seq.size())
    {
        QJsonObject current =
                seq[sequenceIndex].toObject();

        QString type = current["type"].toString();
        QString value = current["value"].toString();

        bool matched = false;

        if (type == "contains")
        {
            matched = buffer.contains(value);
        }
        else if (type == "regex")
        {
            QRegularExpression re(value);
            matched = re.match(buffer).hasMatch();
        }

        if (matched)
        {
            sequenceIndex++;
        }

    }

}

void ProtocolHandler::checkEnd()
{
    QJsonObject endExp = exp["end"].toObject();

    if (endExp.isEmpty()) return;

    QString type = endExp["type"].toString();
    QString value = endExp["value"].toString();

    if (type == "contains")
    {
        if (buffer.contains(value))
        {
            endReceived = true;

            // sequence incomplete

            QJsonArray seq = exp["sequence"].toArray();

            if (sequenceIndex < seq.size())
            {
                QVariantMap m;

                m["stage"] = "sequence";
                m["step"] = sequenceIndex;
                m["expected"] =
                        seq[sequenceIndex].toObject()
                        .toVariantMap()["value"];

                m["actual_buffer"] = buffer.right(200);

                mismatches.append(m);
            }
        }
    }
}

bool ProtocolHandler::isDone(QString &msg) const
{
    bool startOk = true;
    bool endOk = true;

    QJsonValue startExp = exp["start"];

    QJsonValue endExp = exp["end"];

    if (!startExp.isNull())
    {
        startOk = startReceived;
    }

    if (!endExp.isNull())
    {
        endOk = endReceived;
    }

    if (!startOk)
    {
        msg.append("\nstart sequence did not receive");
    }

    if (!endOk)
    {
        msg.append("\nend sequence did not receive");
    }

    int seqSize =
            exp["sequence"]
            .toArray()
            .size();

    return (
                startOk
                && sequenceIndex >= seqSize
                && endOk
                );
}
