#include "protocol_handler.h"


ProtocolHandler::ProtocolHandler(const QJsonObject& expectations,
                                 QString caseFile,
                                 QString jsonObjName)
{
    expStr = jsonObjName;
    json.setJsonFile(caseFile);
    exp = expectations;
}

bool ProtocolHandler::feed(const QByteArray& data, QString& msgError)
{
    try
    {
        QString text = QString::fromUtf8(data);
        bool status = false;

        buffer += text;
        messages.push_back(text);

        QJsonValue startExp = exp["start"];
        QJsonValue endExp = exp["end"];

        if (startExp.isNull()) { startReceived = true; }

        if (!startReceived)
        {
            status = checkStart(msgError);
            if (status == false) { return status; }
        }

        if (startReceived)
        {
            status = checkSequence(msgError);
            if (status == false) { return status; }
        }

        if (endExp.isNull()) { endReceived = true; }

        if (!endReceived)
        {
            status = checkEnd(msgError);
            if (status == false) { return status; }
        }

        return true;
    }
    catch (ErrOpenFile &errOpen)
    {
        msgError = QString("%1, path: %2").arg(errOpen.getMessage(), errOpen.getFilePath());
        return false;
    }
        catch (ErrInJsonSet &jsonSet)
    {
        if (jsonSet.checkErrFromJson())
        {
            msgError = QString("%1. %2 : %3. file: %4")
                                     .arg(jsonSet.getIntro(), jsonSet.getErrMsg(),
                                          jsonSet.getErrFromJson(), jsonSet.getFname());
            return false;
        }
        else
        {
            msgError = QString("%1. %2: \nJSON parameter: %3\nJSON object: %4.\nfile: %5")
                                     .arg(jsonSet.getIntro(), jsonSet.getErrMsg(), jsonSet.getParam(),
                                          jsonSet.getJsonObj(), jsonSet.getFname());
            return false;
        }
    }
}

bool ProtocolHandler::checkStart(QString &msgErr)
{
    if (!buffer.contains("{"))
    {
        msgErr = "missing '{' in received data UART";
        return false;
    }

    int start = buffer.indexOf("{");
    int end = buffer.indexOf("}");

    if (end < 0)
    {
        msgErr = "missing '}' in received data UART";
        return false;
    }

    QString jsonText = buffer.mid(start, end - start + 1);
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8());

    if (!doc.isObject())
    {
        msgErr = "error in first message in received data UART";
        return false;
    }

    QJsonObject msg = doc.object();
    QJsonObject expected = exp["start"].toObject()["fields"].toObject();

    if (expected.isEmpty())
    {
        msgErr = "missing 'start' or 'fields' field";
        return false;
    }

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

    return true;
}

bool ProtocolHandler::checkSequence(QString& msgErr)
{
    QJsonArray seq = exp["sequence"].toArray();
    QString type, value;

    while (sequenceIndex < seq.size())
    {
        QJsonObject current = seq[sequenceIndex].toObject();

        if (current.isEmpty())
        {
            msgErr = "incorrect 'sequence' group";
            return false;
        }

        json.jsonGetStrValue(current, "type", type, "sequence");
        json.jsonGetStrValue(current, "value", value, "sequence");

        bool matched = false;

        if (type == "contains")
        { matched = buffer.contains(value); }
        else if (type == "regex")
        {
            QRegularExpression re(value);
            matched = re.match(buffer).hasMatch();
        }
        else {
            msgErr = QString("unsupported format '%1' in 'sequence' group!").arg(type);
            return false;
        }

        if (!matched)
        {
            msgErr = QString("in received UART data the following string is not found: %1").arg(value);
            return false;
        }
        sequenceIndex++;
    }

    return true;
}

bool ProtocolHandler::checkEnd(QString &msgErr)
{
    QJsonObject endExp = exp["end"].toObject();
    QString type, value;

    if (endExp.isEmpty())
    {
        msgErr = "incorrect 'end' group";
        return false;
    }

    json.jsonGetStrValue(endExp, "type", type, "end");
    json.jsonGetStrValue(endExp, "value", value, "end");

    if (type == "contains")
    {
        if (buffer.contains(value))
        {
            endReceived = true;

            QJsonArray seq = exp["sequence"].toArray();

            if (sequenceIndex < seq.size())
            {
                QVariantMap m;

                m["stage"] = "sequence";
                m["step"] = sequenceIndex;
                m["expected"] = seq[sequenceIndex].toObject()
                        .toVariantMap()["value"];
                m["actual_buffer"] = buffer.right(200);

                mismatches.append(m);
            }
        }
    }
    else
    {
        msgErr = QString("not support %1 in 'end' group").arg(type);
        return false;
    }

    return true;
}

bool ProtocolHandler::isDone(QString &msg) const
{
    bool startOk = true;
    bool endOk = true;

    QJsonValue startExp = exp["start"];
    QJsonValue endExp = exp["end"];

    if (!startExp.isNull())
    { startOk = startReceived; }

    if (!endExp.isNull())
    { endOk = endReceived; }

    if (!startOk)
    { msg.append("\nstart sequence did not receive"); }

    if (!endOk)
    { msg.append("\nend sequence did not receive"); }

    int seqSize = exp["sequence"].toArray().size();

    return (startOk && sequenceIndex >= seqSize && endOk);
}
