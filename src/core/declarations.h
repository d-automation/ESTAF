#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <QByteArray>
#include <QDateTime>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QHash>
#include <QHashIterator>
#include "QLoggingCategory"
#include "QProcess"
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QSettings>
#include <QTextCodec>
#include <QtCore>
#include <QTextStream>
#include <QtSerialPort/QSerialPort>
#include <QVariant>
#include <QVariantList>
#include <QVector>

#include <gtest/gtest.h>

#include "loggingCategories.h"
#include "html_report.h"

struct comSet_t {
        int addrRS485;
        QString name;
        qint32 baudRate;
        QSerialPort::DataBits dataBits;
        QSerialPort::Parity parity;
        QSerialPort::StopBits stopBits;
        QSerialPort::FlowControl flowControl;

        comSet_t()
        {
            addrRS485 = 1;
            name = "";
            baudRate = QSerialPort::Baud9600;
            dataBits = QSerialPort::Data8;
            parity = QSerialPort::NoParity;
            stopBits = QSerialPort::OneStop;
            flowControl = QSerialPort::NoFlowControl;
        }

};
typedef comSet_t comSettings_t;

typedef struct {
    QString MK_elf_file;
    QString MK_map_file;
    QString test_log_path;
    QString stand_report_html_path;
    QString map_parser;
    QString elf_parser;
    QString iar_json_out;
    QString local_python;
} path_t;




#endif // DECLARATIONS_H
