#include "uart.h"

UART::UART()
{
}

UART::~UART()
{
}

void UART::connect(comSettings_t set)
{
    serial.setPortName(set.name);
    serial.setBaudRate(set.baudRate);
    serial.setDataBits(set.dataBits);
    serial.setParity(set.parity);
    serial.setStopBits(set.stopBits);
    serial.setFlowControl(set.flowControl);


    if (!serial.open(QIODevice::ReadWrite))
    {
        throw(ErrOpenFile(QString("error opening com-port: %1").
                        arg(serial.errorString()), set.name));
    }
}

bool UART::send(const QByteArray& data, QString &err)
{
    bool st = false;
    int status = serial.write(data);
    serial.flush();

    if (status == -1)
    { return false; }
    else
    { st = serial.waitForBytesWritten(3000); }

    err = serial.errorString();

    return st;
}

QByteArray UART::receive(int timeoutMs)
{
    QByteArray result;

    while (serial.waitForReadyRead(timeoutMs))
    {
        result += serial.readAll();
    }

    return result;
}

void UART::close()
{
    serial.close();
}
