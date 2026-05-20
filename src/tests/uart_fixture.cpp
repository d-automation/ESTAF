#include "uart_fixture.h"
#include "../core/cli_parser.h"
#include "../core/html_report.h"

UART UARTFixture::uart;
comSettings_t UARTFixture::comPortSettings;
Stats_t UARTFixture::stats;
path_t UARTFixture::path;
UART_cmdReadMemory_t UARTFixture::iData;
char UARTFixture::cmdDataBuf[UART_DATA_MAX_BYTES];
QVector<QString> UARTFixture::struct_names;
JsonProcessor UARTFixture::json;
QString UARTFixture::logFile;
extern TestOptions g_options;

int UARTFixture::convert_map_file()
{
    QProcess process;
    QString program = "python";

    QStringList arguments;
    arguments << path.map_parser
              << "-f" << path.MK_map_file
              << "-s" << "ENTRY LIST"
              << "-o" << path.iar_json_out;

    process.start(program, arguments);

    if (!process.waitForFinished(3000))
    {
        writeToLog(QString("timeout processing scipt %1").arg(path.map_parser), false);
        return 1;
    }

    int exitCode = process.exitCode();

    if (exitCode != 0) {
        QString msg = QString("scipt %1 return error code %2: %3")
                .arg(path.map_parser, QString(exitCode),
                     QString::fromUtf8(process.readAllStandardError()));

        writeToLog(msg, false);
        return 1;
    }

   return 0;
}

int UARTFixture::readConfig()
{
    JsonProcessor json;
    QJsonObject jsonObj;
    QString jsonUart = "UART";
    QString jsonPath = "Path";

    try
    {
        json.openJsonFile("../config.json", jsonObj);
        QJsonObject objUart = jsonObj.value(jsonUart).toObject();
        QJsonObject objPath = jsonObj.value(jsonPath).toObject();

        json.jsonSetComPortSettings(jsonUart, objUart, comPortSettings);
        json.jsonGetStrValue(objPath, "MK_elf_file", path.MK_elf_file, jsonPath);
        json.jsonGetStrValue(objPath, "MK_map_file", path.MK_map_file, jsonPath);
        json.jsonGetStrValue(objPath, "test_log_path", path.test_log_path, jsonPath);
        json.jsonGetStrValue(objPath, "stand_report_html_path", path.stand_report_html_path, jsonPath);
        json.jsonGetStrValue(objPath, "map_parser", path.map_parser, jsonPath);
        json.jsonGetStrValue(objPath, "elf_parser", path.elf_parser, jsonPath);
        json.jsonGetStrValue(objPath, "iar_json_out", path.iar_json_out, jsonPath);
        json.jsonGetStrValue(objPath, "local_python", path.local_python, jsonPath);
    }
        catch (ErrOpenFile &errOpen)
    {
        QString msg = QString("%1, path: %2").arg(errOpen.getMessage(), errOpen.getFilePath());
        writeToLog(msg, false);
        return 1;
    }
        catch (ErrInJsonSet &jsonSet)
    {
        if (jsonSet.checkErrFromJson())
        {
            QString msg = QString("%1. %2 : %3. file: %4")
                                     .arg(jsonSet.getIntro(), jsonSet.getErrMsg(),
                                          jsonSet.getErrFromJson(), jsonSet.getFname());
            writeToLog(msg, false);
            return 1;
        }
        else
        {
            QString msg = QString("%1. %2: \nJSON parameter: %3\nJSON object: %4.\nfile: %5")
                                     .arg(jsonSet.getIntro(), jsonSet.getErrMsg(), jsonSet.getParam(),
                                          jsonSet.getJsonObj(), jsonSet.getFname());
            writeToLog(msg, false);
            return 1;
        }
    }

    return 0;
}

void UARTFixture::SetUpTestSuite()
{
    Logger::setupLog();
    memset(&stats, 0, sizeof(stats));

    int fail = UARTFixture::readConfig();
    if (fail == true) { return; }
    fail = UARTFixture::convert_map_file();
    if (fail == true) { return; }

    logFile = QString("%1test_cases.log").arg(path.test_log_path);
    QFile::remove(logFile);

    try
    {
        uart.connect(comPortSettings);
    }
    catch (ErrOpenFile &errOpen)
    {
        QString msg = QString("%1, path: %2").arg(errOpen.getMessage(), errOpen.getFilePath());
        writeToLog(msg, false);
        ASSERT_TRUE(false);
        return;
    }
}

void UARTFixture::TearDownTestSuite()
{
    uart.close();

    QString curTime1 = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
    QString htmlFile = path.stand_report_html_path;
    Logger::appendSummary(logFile, stats.total, stats.passed, stats.failed);
    HtmlReport::generate(logFile, htmlFile, curTime1);
}

TEST_P(UARTFixture, JsonCase)
{
    TestCaseParam param = GetParam();
    QString caseFile = param.path;
    runCase(caseFile);
}


bool UARTFixture::loadTestCase(const QString& caseFile,
                      QJsonObject& cfg, QJsonObject &meta)
{
    json.openJsonFile(caseFile, cfg);
    meta = cfg["meta"].toObject();

    if (cfg.isEmpty() || meta.isEmpty())
    {
        writeToLog(QString("%1, path: %2").arg("incorrect json file", caseFile), false);
        return false;
    }

    return true;
}


void UARTFixture::updateStats(bool passed)
{
    stats.total++;

    if (passed) { stats.passed++; }
    else { stats.failed++; }

    EXPECT_TRUE(passed);
}

bool UARTFixture::sendCommand(const QByteArray &data)
{
    QString err;

    if (!uart.send(data, err))
    {
        writeToLog(QString("UART send failed: %1").arg(err), false);
        return false;
    }

    return true;
}

QByteArray UARTFixture::receiveResponse(const QJsonObject &cfg)
{
    int timeout = cfg["timeout_msec"].toInt();
    return uart.receive(timeout);
}


bool UARTFixture::loadBinaryFile(const QString& fileName, QByteArray& data)
{
    QFile binaryFile(fileName);

    if (!binaryFile.open(QIODevice::ReadOnly))
    {
        writeToLog(QString("command binary not found: %1").arg(fileName), false);
        return false;
    }

    data = binaryFile.readAll();
    return true;
}

bool UARTFixture::validateTextResponse(const QByteArray& rx,
                              const QJsonObject& cfg, QString caseFile)
{
    QString protocolMsg;
    QString error;
    bool passed = false;

    QJsonObject exp = cfg["expectations"].toObject();

    if (exp.isEmpty())
    {
        writeToLog(QString("'expectations' field not found in %1").arg(caseFile), false);
        passed = false; return passed;
    }

    ProtocolHandler handler(exp);

    handler.feed(rx);

    passed = handler.isDone(protocolMsg);

    if (!passed)
    { error = QString("Protocol validation failed:\n%1").arg(protocolMsg); }

    Logger::saveTestLog(logFile, handler, cfg, passed, error);

    return passed;
}

bool UARTFixture::runTextCase(const QJsonObject& cfg, QString caseFile, QString jsonObjName)
{
    QByteArray tx;
    QString binaryFname;
    json.jsonGetStrValue(cfg, "binary", binaryFname, jsonObjName);

    if (!loadBinaryFile(binaryFname, tx)) { return false; }
    if (!sendCommand(tx)) { return false; }

    QByteArray rx = receiveResponse(cfg);

    return validateTextResponse(rx, cfg, caseFile);
}


bool UARTFixture::prepareBinaryCommand(const QJsonObject& cfg, QByteArray& data, QString& caseFile)
{
    QJsonObject meta, paramTestCase, map_json, structObj_map;
    bool ok;

    QString map_fname = QString("%1ENTRY_LIST_map.json").arg(path.iar_json_out);
    json.openJsonFile(map_fname, map_json);

    meta = cfg["meta"].toObject();
    paramTestCase = meta["parameters"].toObject();

    QString struct_name;
    json.jsonGetStrValue(paramTestCase, "struct_name", struct_name, "parameters");

    if (meta.isEmpty() || paramTestCase.isEmpty())
    {
        writeToLog(QString("'meta' or 'parameters' field not found"), false);
        return false;
    }

    iData.version = json.jsonGetIntValue(paramTestCase, "version", "parameters");

    bool addr_user_set = json.jsonGetBoolValue(paramTestCase, "addr_user_set", "parameters");
    bool size_user_set = json.jsonGetBoolValue(paramTestCase, "size_user_set", "parameters");

    if (addr_user_set == false || size_user_set == false)
    {
        if (!map_json.contains(struct_name))
        {
            writeToLog(QString("'%1' field not found in json file: %2")
                       .arg(struct_name, map_fname), false);
            return false;
        }

        structObj_map = map_json.value(struct_name).toObject();

        if (structObj_map.isEmpty())
        {
            writeToLog(QString("'%1' field not found in json file: %2")
                       .arg(struct_name, map_fname), false);
            return false;
        }
    }

    if (addr_user_set)
    {
        QString addr;
        json.jsonGetStrValue(paramTestCase, "address", addr, "parameters");
        iData.addr_begin = addr.toUInt(&ok, 16);
        if (!ok)
        {
            writeToLog(QString("'addr' field has incorrect format in json file: %2")
                       .arg(struct_name, caseFile), false);
            return false;
        }
    }
    else
    {
        QString addrStr;
        json.jsonGetStrValue(structObj_map, "address", addrStr, struct_name);
        bool addrOk = false;
        iData.addr_begin = addrStr.toUInt(&addrOk, 16);
        if (!addrOk)
        {
            writeToLog(QString("'address' field has incorrect format in json file: %2")
                       .arg(struct_name, map_fname), false);
            return false;
        }
    }

    if (size_user_set)
    { iData.size_bytes =  json.jsonGetIntValue(paramTestCase, "size_bytes", "parameters"); }
    else
    { iData.size_bytes = json.jsonGetIntValue(structObj_map, "size_bytes", struct_name); }

    if (iData.size_bytes >= UART_DATA_MAX_BYTES)
    {
        writeToLog(QString("structure too big"), false);
        return false;
    }

    bool binaryFile = json.jsonGetBoolValue(paramTestCase, "cmd_from_binary_file", "parameters");
    if (binaryFile)
    {
        QString binaryPath;
        json.jsonGetStrValue(cfg, "binary", binaryPath, "");
        bool ok = loadBinaryFile(binaryPath, data);
        return ok;
    }

    formCmdDataInBuffer();

    data = QByteArray::fromRawData(cmdDataBuf, UART_DATA_MAX_BYTES);

    return true;
}

bool UARTFixture::validateStructField(const QByteArray& rxData,
                                      const QJsonObject& fieldCfg,
                                      const QJsonObject& structJson,
                                      QString& structJsonFname,
                                      QString& errMsg,
                                      QString& errMsgMismatch,
                                      QString caseFile,
                                      QString jsonObjName)
{
    bool found = false;
    QJsonObject fieldInfo;
    QString fieldName, fieldType, op;
    bool isArray = false;
    QJsonArray fields;

    json.jsonGetStrValue(fieldCfg, "field", fieldName, jsonObjName);
    json.jsonGetStrValue(fieldCfg, "type", fieldType, jsonObjName);
    json.jsonGetStrValue(fieldCfg, "op", op, jsonObjName);
    double expectValue = json.jsonGetDoubleValue(fieldCfg, "expect", jsonObjName);

    // !!! array field is not necessary so not nececarry using JsonProcessor
    isArray = fieldCfg["array"].toBool(false);
    fields = structJson["fields"].toArray();

    for (const QJsonValue& v : qAsConst(fields))
    {
        QJsonObject obj = v.toObject();

        if (obj["name"].toString() == fieldName)
        {
            fieldInfo = obj;
            found = true;
            break;
        }
    }

    if (!found)
    {
        errMsg = QString("field '%1' not found in struct json").arg(fieldName);
        return false;
    }

    // offset and size from ELF JSON
    json.setJsonFile(structJsonFname);
    int offset = json.jsonGetIntValue(fieldInfo, "offset", "");
    int size = json.jsonGetIntValue(fieldInfo, "size", "");
    json.setJsonFile(caseFile);

    if ((offset + size) > rxData.size())
    {
        errMsg = QString("field '%1' out of bounds (offset=%2 size=%3 rx_size=%4)")
                     .arg(fieldName).arg(offset).arg(size).arg(rxData.size());
        return false;
    }

    // ARRAY CHECK
    if (isArray)
    {
        // exclude list
        QVector<int> excludeIdx;
        QJsonArray excludeArr = fieldCfg["exclude"].toArray();

        for (const QJsonValue& v : qAsConst(excludeArr))
        { excludeIdx.push_back(v.toInt()); }

        // element array size
        int elemSize = 1;

        if (fieldType == "uint8"  || fieldType == "int8") { elemSize = 1; }
        else if (fieldType == "uint16" || fieldType == "int16") { elemSize = 2; }
        else if (fieldType == "uint32" || fieldType == "int32" || fieldType == "float") { elemSize = 4; }
        else if (fieldType == "uint64" || fieldType == "int64" || fieldType == "double") { elemSize = 8; }
        else
        {
            errMsg = QString("unsupported array type '%1'").arg(fieldType);
            return false;
        }

        int arrayCount = size / elemSize;

        // iterate array
        for (int i = 0; i < arrayCount; i++)
        {
            bool skip = false;
            for (int ex : excludeIdx)
            {  if (ex == i) { skip = true; break; } }

            if (skip) { continue; }

            // read value by type
            double actual = 0;
            const char* ptr = rxData.constData() + offset + i * elemSize;

            if (fieldType == "uint8") actual = *(reinterpret_cast<const uint8_t*>(ptr));
            else if (fieldType == "int8") actual = *(reinterpret_cast<const int8_t*>(ptr));
            else if (fieldType == "uint16") actual = *(reinterpret_cast<const uint16_t*>(ptr));
            else if (fieldType == "int16") actual = *(reinterpret_cast<const int16_t*>(ptr));
            else if (fieldType == "uint32") actual = *(reinterpret_cast<const uint32_t*>(ptr));
            else if (fieldType == "int32") actual = *(reinterpret_cast<const int32_t*>(ptr));
            else if (fieldType == "uint64") actual = static_cast<double>(*(reinterpret_cast<const uint64_t*>(ptr)));
            else if (fieldType == "int64") actual = static_cast<double>(*(reinterpret_cast<const int64_t*>(ptr)));
            else if (fieldType == "float") actual = *(reinterpret_cast<const float*>(ptr));
            else if (fieldType == "double") actual = *(reinterpret_cast<const double*>(ptr));

            // compare
            bool ok = false;
            if (op == "==") ok = (actual == expectValue);
            else if (op == "!=") ok = (actual != expectValue);
            else if (op == ">") ok = (actual > expectValue);
            else if (op == "<") ok = (actual < expectValue);
            else if (op == ">=") ok = (actual >= expectValue);
            else if (op == "<=") ok = (actual <= expectValue);

            if (!ok)
            {
                errMsgMismatch = QString("array check failed: %1[%2] actual=%3 expect %4 %5")
                             .arg(fieldName).arg(i).arg(actual).arg(op).arg(expectValue);
                return false;
            }
        }
        return true;
    }

    // SINGLE VARIABLE CHECK
    const char* ptr = rxData.constData() + offset;

    double actual = 0;

    if (fieldType == "uint8") { actual = *(reinterpret_cast<const uint8_t*>(ptr)); }
    else if (fieldType == "int8") { actual = *(reinterpret_cast<const int8_t*>(ptr)); }
    else if (fieldType == "uint16") { actual = *(reinterpret_cast<const uint16_t*>(ptr)); }
    else if (fieldType == "int16") { actual = *(reinterpret_cast<const int16_t*>(ptr)); }
    else if (fieldType == "uint32") { actual = *(reinterpret_cast<const uint32_t*>(ptr)); }
    else if (fieldType == "int32") { actual = *(reinterpret_cast<const int32_t*>(ptr)); }
    else if (fieldType == "uint64") { actual = static_cast<double>(*(reinterpret_cast<const uint64_t*>(ptr))); }
    else if (fieldType == "int64") { actual = static_cast<double>(*(reinterpret_cast<const int64_t*>(ptr))); }
    else if (fieldType == "float") { actual = *(reinterpret_cast<const float*>(ptr)); }
    else if (fieldType == "double") { actual = *(reinterpret_cast<const double*>(ptr)); }
    else
    {
        errMsg = QString("unsupported field type '%1'").arg(fieldType);
        return false;
    }

    bool ok = false;
    if (op == "==") { ok = (actual == expectValue); }
    else if (op == "!=") { ok = (actual != expectValue); }
    else if (op == ">") { ok = (actual > expectValue); }
    else if (op == "<") { ok = (actual < expectValue); }
    else if (op == ">=") { ok = (actual >= expectValue); }
    else if (op == "<=") { ok = (actual <= expectValue); }

    if (!ok)
    {
        errMsgMismatch = QString("field check failed: %1 actual=%2 expect %3 %4")
                     .arg(fieldName).arg(actual).arg(op).arg(expectValue);
        return false;
    }
    return true;
}

bool UARTFixture::validateBinaryResponse(const QByteArray& rx, const QJsonObject& cfg,
                                         const QJsonObject& structJson, QString &structJsonFname,
                                         QString caseFile)
{
    QString json_checks_field = "struct_check";
    QJsonArray checks = cfg[json_checks_field].toArray();
    QString err, errMsgMismatch;
    bool passed = false;

    if (checks.isEmpty())
    {
        writeToLog(QString("'struct_check' field not found in %1").arg(caseFile), false);
        return false;
    }

    for (const QJsonValue& v: qAsConst(checks))
    {
        passed = validateStructField(rx, v.toObject(), structJson, structJsonFname,
                                     err, errMsgMismatch, caseFile,
                                     json_checks_field);
        if (!passed) { writeToLog(QString("%1\n%2").arg(err, errMsgMismatch) , false); break; }
    }

    Logger::saveTestLog(logFile, cfg, passed, err, errMsgMismatch);
    return true;
}

bool UARTFixture::runBinaryCase(const QJsonObject& cfg, const QJsonObject& meta,
                                QString caseFile)
{
    QByteArray tx;
    QString struct_name, struct_type, structJsonFileName;
    QJsonObject param, structJson;

    // find struct_name from test_case
    QString parameters = "parameters";
    param = meta[parameters].toObject();
    if (param.isEmpty())
    {
        writeToLog(QString("'parameters' field not found in %1").arg(caseFile), false);
        return false;
    }

    json.jsonGetStrValue(param, "struct_name", struct_name, parameters);
    json.jsonGetStrValue(param, "struct_type", struct_type, parameters);

    structJsonFileName = QString("%1%2.json").arg(path.iar_json_out, struct_type);

    // extract from elf about struct info
    if (!struct_names.contains(struct_name))
    {
        struct_names.push_back(struct_name);
        bool fail = extract_struct_from_elf(struct_type);
        if (fail == true) { return false; }
    }

    // open json-file with specified struct info and get json object
    json.openJsonFile(structJsonFileName, structJson);

    if (!prepareBinaryCommand(cfg, tx, caseFile)) { return false; }
    if (!sendCommand(tx)) { return false; }

    QByteArray rx = receiveResponse(cfg);
    return validateBinaryResponse(rx, cfg, structJson, structJsonFileName, caseFile);
}

void UARTFixture::runCase(const QString& caseFile)
{
    QJsonObject cfg, meta;
    json.setJsonFile(caseFile);

    try
    {
        if (!loadTestCase(caseFile, cfg, meta)) { return; }

        bool passed = false;
        QString rx_data_type;
        json.jsonGetStrValue(meta, "input_data_type", rx_data_type, "meta");

        if (rx_data_type == "text")
        { passed = runTextCase(cfg, caseFile, "meta"); }
        else if (rx_data_type == "binary")
        { passed = runBinaryCase(cfg, meta, caseFile); }
        else
        {
            writeToLog(QString("unknown input_data_type (%1) in file: %2")
                       .arg(rx_data_type, caseFile), false);
            passed = false;
            updateStats(passed);
            return;
        }

        updateStats(passed);

        if (g_options.delayMs > 0)
        { QThread::msleep(g_options.delayMs); }
    }
    catch (ErrOpenFile &errOpen)
    {
        QString msg = QString("%1, path: %2").arg(errOpen.getMessage(), errOpen.getFilePath());
        writeToLog(msg, false);
    }
        catch (ErrInJsonSet &jsonSet)
    {
        if (jsonSet.checkErrFromJson())
        {
            QString msg = QString("%1. %2 : %3. file: %4")
                                     .arg(jsonSet.getIntro(), jsonSet.getErrMsg(),
                                          jsonSet.getErrFromJson(), jsonSet.getFname());
            writeToLog(msg, false);
        }
        else
        {
            QString msg = QString("%1. %2: \nJSON parameter: %3\nJSON object: %4.\nfile: %5")
                                     .arg(jsonSet.getIntro(), jsonSet.getErrMsg(), jsonSet.getParam(),
                                          jsonSet.getJsonObj(), jsonSet.getFname());
            writeToLog(msg, false);
        }
    }
}

void UARTFixture::serializeCmdDataBuf(void *dataStruct, int countBytes)
{
    UART_cmdWord_u cmdWord;

    cmdWord.all = 0;
    cmdWord.bit.cmdCode = UART_cmdReadMemory;
    cmdWord.bit.countBytes = countBytes;

    int cntBytesAll = CMD_WORD_SIZE + cmdWord.bit.countBytes;

    memcpy(cmdDataBuf, &cmdWord.all, CMD_WORD_SIZE);
    memcpy(cmdDataBuf + CMD_WORD_SIZE, dataStruct,
           cmdWord.bit.countBytes - 1);

    quint8 crc = crc8(cmdDataBuf, cntBytesAll - 1);
    cmdDataBuf[cntBytesAll - 1] = crc;
}


void UARTFixture::formCmdDataInBuffer()
{
    int countBytesCmdData = sizeof(UART_cmdReadMemory_t) + 1;
    serializeCmdDataBuf(&iData, countBytesCmdData);
}

void UARTFixture::TearDown()
{
}

int UARTFixture::extract_struct_from_elf(QString structTypeName)
{
    QProcess process;
    QString program = path.local_python;

    QStringList arguments;
    arguments << path.elf_parser
              << "-f" << path.MK_elf_file
              << "-t" << structTypeName
              << "-o" << path.iar_json_out;

    process.start(program, arguments);

    if (!process.waitForFinished(3000))
    {
        writeToLog(QString("timeout script processig: %1").arg(path.elf_parser), false);
        return 1;
    }

    int exitCode = process.exitCode();

    if (exitCode != 0) {
        QString msg = QString("scipt %1 return error code %2: %3")
                .arg(path.elf_parser, QString(exitCode),
                     QString::fromUtf8(process.readAllStandardError()));

        writeToLog(msg, false);
        return 1;
    }

   return 0;
}

void UARTFixture::writeToLog(const QString& errMsg, bool passed)
{
    if (!passed)
    {
        qDebug(logCritical()) << errMsg << Qt::endl;
        Logger::writeToConsol(QString("\nERROR: %1\n").arg(errMsg));
    }
    else {
    }
}
