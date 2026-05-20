#include "json_processor.h"

JsonProcessor::JsonProcessor()
{

}

void JsonProcessor::setJsonFile(QString jsonPath_)
{
   jsonPath = jsonPath_;
}

void JsonProcessor::openJsonFile(QString jsonPath, QJsonObject &jsonObj)
{
    QByteArray bytes;

    QFile file(jsonPath);
    if (file.open( QIODevice::ReadOnly))
    {
        bytes = file.readAll();
        file.close();
    }
    else
    { throw(ErrOpenFile(jsonPath, "file not found")); }

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(bytes, &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
    { throw(ErrInJsonSet(jsonPath, jsonError.errorString())); return; }
    else
    {
        if (doc.isObject())
        { jsonObj = doc.object(); }
        else
        { throw(ErrInJsonSet(jsonPath, "incorrect json format: " + jsonPath)); }
    }
}

void JsonProcessor::saveJsonDataInFile(QString jsonPath, QJsonObject jsonObj)
{
    QJsonDocument document;
    document.setObject(jsonObj);
    QByteArray bytes = document.toJson( QJsonDocument::Indented );
    QFile file(jsonPath);
    if( file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        QTextStream iStream( &file );
        iStream.setCodec( "utf-8" );
        iStream << bytes;
        file.close();
    }
    else
    { throw(ErrOpenFile(jsonPath, "file not found")); }
}

/* common functions */
void JsonProcessor::jsonConvertStruct(QString jsonObjName, QJsonObject obj,
                                QString jsonParamName, QVector<QString> *setToStructArr, int arrSize)
{
    setToStructArr->clear();
    QJsonArray *param = new QJsonArray;
    *param = obj[jsonParamName].toArray();
    if (param->size() != arrSize)
    {
        throw(ErrInJsonSet(jsonPath, jsonObjName, jsonParamName,
                         "parameter is missing or elements count in parameter must be equal " +
                           QString::number(arrSize)));
    }
    else
    {
        for (int i = 0; i < arrSize; i++)
        { setToStructArr->push_back(param->at(i).toString()); }
    }
}

void JsonProcessor::jsonConvertStruct(QJsonObject obj, QString jsonParamName,
                                      QVector<QString> *setToStructArr)
{
    setToStructArr->clear();
    QJsonArray *param = new QJsonArray;
    *param = obj[jsonParamName].toArray();
    for (int i = 0; i < param->size(); i++)
    { setToStructArr->push_back(param->at(i).toString()); }
}

void JsonProcessor::jsonConvertStruct(QJsonObject obj,
                                QString jsonParamName, QVector<double> *setToStructArr)
{
    setToStructArr->clear();
    QJsonArray *param = new QJsonArray;
    *param = obj[jsonParamName].toArray();
    for (int i = 0; i < param->size(); i++)
    { setToStructArr->push_back(param->at(i).toDouble()); }
}

void JsonProcessor::jsonConvertStruct(QString jsonObjName, QJsonObject obj,
                                QString jsonParamName, double *setToStructArr, int arrSize)
{
    QJsonArray param = obj[jsonParamName].toArray();
    if (param.size() != arrSize)
    { throw(ErrInJsonSet(jsonPath, jsonObjName, jsonParamName,
                         "parameter is missing or elements count in parameter must be equal " + QString::number(arrSize))); }
    else
    {
        for (int i = 0; i < arrSize; i++)
        { setToStructArr[i] = param[i].toDouble(); }
    }
}

void JsonProcessor::jsonConvertStruct(QString jsonObjName, QJsonObject obj,
                                QString jsonParamName, bool *setToStructArr, int arrSize)
{
    QJsonArray param = obj[jsonParamName].toArray();
    if (param.size() != arrSize)
    { throw(ErrInJsonSet(jsonPath, jsonObjName, jsonParamName,
                         "parameter is missing or elements count in parameter must be equal " + QString::number(arrSize))); }
    else
    {
        for (int i = 0; i < arrSize; i++)
        { setToStructArr[i] = param[i].toBool(); }
    }
}

bool JsonProcessor::jsonGetBoolValue(QJsonObject obj, QString param, QString jsonGeneral)
{
    QJsonValue val = obj[param];
    if (val == QJsonValue::Null)
    { throw(ErrInJsonSet(jsonPath, jsonGeneral, param, "parameter is missing")); }
    return val.toBool(false);
}

void JsonProcessor::jsonGetStrValue(QJsonObject obj, QString paramName, QString &paramValue, QString jsonObjName)
{
    QString val = obj[paramName].toString();
    if (val == NULL)
    { throw(ErrInJsonSet(jsonPath, jsonObjName, paramName, "parameter is missing")); }
    else
    { paramValue = val; }
}

int JsonProcessor::jsonGetIntValue(QJsonObject obj, QString paramName, QString jsonObjName)
{
    QJsonValue val = obj[paramName];
    if (val == QJsonValue::Null)
    { throw(ErrInJsonSet(jsonPath, jsonObjName, paramName, "parameter is missing")); }
    return val.toInt();
}

double JsonProcessor::jsonGetDoubleValue(QJsonObject obj, QString paramName, QString jsonObjName)
{
    QJsonValue val = obj[paramName];
    if (val == QJsonValue::Null)
    { throw(ErrInJsonSet(jsonPath, jsonObjName, paramName, "parameter is missing")); }
    return val.toDouble();
}


void JsonProcessor::jsonSetComPortSettings(QString jsonObjName, QJsonObject obj, comSettings_t &com)
{
    com.addrRS485 = obj["RS485_address"].toInt();
    if (com.addrRS485 < 1 ||
            com.addrRS485 > 254)
    { throw(ErrInJsonSet(jsonPath, jsonObjName, "RS485_address", "paremeter is missing")); }

    com.name = obj["COM_port"].toString();
    if (com.name == NULL) { throw(ErrInJsonSet(jsonPath, jsonObjName, "COM_port", "paremeter is missing")); }

    com.baudRate = (quint32)obj["COM_baudRate"].toInt();
    if (com.baudRate == 0) { throw(ErrInJsonSet(jsonPath, jsonObjName, "COM_baudRate", "paremeter is missing")); }

    com.dataBits = (QSerialPort::DataBits)obj["COM_bits"].toInt();
    if (com.dataBits < QSerialPort::Data5 ||
            com.dataBits > QSerialPort::Data8)
    { throw(ErrInJsonSet(jsonPath, jsonObjName, "COM_bits", "paremeter is missing")); }

    com.parity = (QSerialPort::Parity)obj["COM_parity"].toInt();
    if (com.parity < QSerialPort::UnknownParity ||
            com.parity == 1 ||
            com.parity > QSerialPort::MarkParity)
    { throw(ErrInJsonSet(jsonPath, jsonObjName, "COM_parity", "paremeter is missing")); }

    com.stopBits = (QSerialPort::StopBits)obj["COM_stopBits"].toInt();
    if (com.stopBits < QSerialPort::OneStop ||
            com.stopBits > QSerialPort::OneAndHalfStop)
    { throw(ErrInJsonSet(jsonPath, jsonObjName, "COM_stopBits", "paremeter is missing")); }

    com.flowControl = (QSerialPort::FlowControl)obj["COM_flowCotrol"].toInt();
    if (com.flowControl < QSerialPort::NoFlowControl ||
            com.flowControl > QSerialPort::SoftwareControl)
    { throw(ErrInJsonSet(jsonPath, jsonObjName, "COM_flowControl", "paremeter is missing")); }
}

void JsonProcessor::jsonSaveComPortSettings(QJsonObject &obj, comSettings_t &com)
{
    obj.insert("RS485_address", com.addrRS485);
    obj.insert("COM_port", com.name);
    obj.insert("COM_baudRate", com.baudRate);
    obj.insert("COM_bits", com.dataBits);
    obj.insert("COM_parity", com.parity);
    obj.insert("COM_stopBits", com.stopBits);
    obj.insert("COM_flowControl", com.flowControl);
}
