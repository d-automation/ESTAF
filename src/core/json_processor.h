#ifndef JSONPROCESSOR_H
#define JSONPROCESSOR_H

#include "exceptions_handle.h"

class JsonProcessor
{

public:
    JsonProcessor();

    virtual void setJsonFile(QString jsonPath_);
    virtual void openJsonFile(QString jsonPath, QJsonObject &jsonObj);
    virtual void saveJsonDataInFile(QString jsonPath, QJsonObject jsonObj);

    virtual void jsonConvertStruct(QString jsonObjName, QJsonObject obj,
                                QString jsonParamName, QVector<QString> *setToStructArr, int arrSize);
    virtual void jsonConvertStruct(QJsonObject obj,
                                QString jsonParamName, QVector<QString> *setToStructArr);
    virtual void jsonConvertStruct(QJsonObject obj,
                                QString jsonParamName, QVector<double> *setToStructArr);
    virtual void jsonConvertStruct(QString jsonObjName, QJsonObject obj,
                                QString jsonParamName, double *setToStructArr, int arrSize);
    virtual void jsonConvertStruct(QString jsonObjName, QJsonObject obj,
                                QString jsonParamName, bool *setToStructArr, int arrSize);

    virtual bool jsonGetBoolValue(QJsonObject obj, QString param, QString jsonPSgeneral);

    virtual void jsonSetComPortSettings(QString jsonObjName, QJsonObject obj, comSettings_t &com);
    virtual void jsonSaveComPortSettings(QJsonObject &obj, comSettings_t &com);

    virtual void jsonGetStrValue(QJsonObject obj, QString paramName, QString &paramValue, QString jsonObjName);


private:
    QString jsonPath = "";
};

#endif // JSONPROCESSOR_H
