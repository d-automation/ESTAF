#pragma once

#include "exceptions_handle.h"

class UART
{
public:
    UART();
    ~UART();

public:
    void connect(comSettings_t set);

    bool send(const QByteArray& data, QString &err);

    QByteArray receive(int timeoutMs);

    void close();

private:
    QSerialPort serial;
};
