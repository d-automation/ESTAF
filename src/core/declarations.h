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
#include <QtEndian> // Для qbswap

#include <gtest/gtest.h>

#include "loggingCategories.h"
#include "html_report.h"
#include "crc8.h"

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



#pragma pack(push,1)
typedef struct
{
    uint8_t replyAddrLength:	2;		///< reply address length field (for path addressing):
                                        ///< \arg 0b00 - 0 bytes,
                                        ///< \arg 0b01 - 4 bytes,
                                        ///< \arg 0b10 - 8 bytes,
                                        ///< \arg 0b11 - 12 bytes
    uint8_t increment:			1;		///< 1 - incrementing address
    uint8_t reply:				1;		///< 1 - transmit reply, 0 - not transmit reply
    uint8_t checkData:			1;		///< 1 - checking data before write
    uint8_t operationType:		1;		///< 1 - write data, 0 - read data
    uint8_t packetTypeField:	2;		///< 0b00 - logical address, 0b01 - physical address - for MIL layer
                                        ///< 0b01 - command, 0b00 - reply for RMAP layer

} RMAP_instruct_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef union
{
    uint8_t all;
    RMAP_instruct_t bit;
} RMAP_instruct_u;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint8_t targetLogAddr;
    uint8_t protocolID;
    RMAP_instruct_u instruct;
    uint8_t key;
    uint8_t initiatorLogAddr;
    uint8_t transactID[2];
    uint8_t addr[5];
    uint8_t dataLength[3];
    uint8_t headerCRC;
} RMAP_cmdWriteReadHeadLogAddr_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint8_t	initiatorLogAddr;
    uint8_t protocolID;
    uint8_t instruct;
    uint8_t status;
    uint8_t targetLogAddr;
    uint16_t transactID;
    uint8_t headerCRC;
} RMAP_WriteReplyHead_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint8_t	initiatorLogAddr;
    uint8_t protocolID;
    uint8_t instruct;
    uint8_t status;
    uint8_t targetLogAddr;
    uint16_t transactID;
    uint8_t reserved;
    uint32_t dataLength;
    uint8_t headerCRC;
} RMAP_ReadReplyHead_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint8_t targetSpwAddr[2];
    uint8_t targetLogAddr;
    uint8_t protocolID;
    RMAP_instruct_u instruct;
    uint8_t key;
    uint8_t replyAddr[4];
    uint8_t initiatorLogAddr;
    uint16_t transactID;
    uint8_t extendAddr;
    uint32_t addr;
    uint8_t dataLength[3];
    uint8_t headerCRC;
} RMAP_cmdWriteReadHeadPathAddr_t;
#pragma pack(pop)

typedef enum
{
    RMAP_success = 0,					///< 00
    RMAP_errGeneral,					///< 01
    RMAP_errUnusePackType,				///< 02
    RMAP_errInvalKey,					///< 03
    RMAP_errInvalDataCRC,				///< 04
    RMAP_errDataEarlyEOP,				///< 05
    RMAP_errTooMuchData,				///< 06
    RMAP_errDataEEP,					///< 07
    RMAP_errReserved,					///< 08
    RMAP_errVerifyBufOver,				///< 09
    RMAP_errCmdNotImplement,			///< 10
    RMAP_errDataLengthErr,				///< 11
    RMAP_errInvalTargLogAddr,			///< 12
    RMAP_errHeaderEarlierEOP,			///< 13
    RMAP_errInvalHeaderCRC,				///< 14
    RMAP_errNotRMAP,					///< 15
    RMAP_errReplyTimeout,				///< 16
    RMAP_errUnknownTransactID,			///< 17
    RMAP_spwReconnectFailed,			///< 18
    RMAP_errInputParam,					///< 19
    RMAP_unknownInitiatorLogAddrInReply,///< 20
    RMAP_errTxTimeout,					///< 21
    RMAP_errRxReplyTimeout, 			///< 22
    COUNT_ERR_MAX_ELEMENTS_RMAP = 		30
} RMAP_errFlag_en;

/**
 * \brief structure with exchange error status counters
 *  according to RMAP protocol (ECSS-E-ST-50-52C), table 5-4
 */
#pragma pack(push,1)
typedef struct
{
    uint16_t success; 					///< 00
    uint16_t general; 					///< 01
    uint16_t unusePackType;				///< 02
    uint16_t invalKey;					///< 03
    uint16_t invalDataCRC;				///< 04
    uint16_t dataEarlyEOP;				///< 05
    uint16_t tooMuchData;				///< 06
    uint16_t dataEEP;					///< 07
    uint16_t reserved;					///< 08
    uint16_t verifyBufOver;				///< 09
    uint16_t cmdNotImplement;			///< 10
    uint16_t dataLengthErr;				///< 11
    uint16_t invalTargLogAddr;			///< 12
    uint16_t headerEarlierEOP;			///< 13
    uint16_t invalHeaderCRC;			///< 14
    uint16_t notRMAP;					///< 15
    uint16_t replyTimeout;				///< 16
    uint16_t unknownTransactID;			///< 17
    uint16_t spwReconnectFailed;		///< 18
    uint16_t invalInputParam;			///< 19
    uint16_t unknownInitiatorLogAddrInReply; ///< 20
    uint16_t txTimeout;					///< 21
    uint16_t rxReplyTimeout;			///< 22
    uint16_t reserve[7];
} RMAP_countErr_t;
#pragma pack(pop)

typedef enum
{
    rmapNone = 0,
    rmapRunTxCmd,
    rmapTxCmdCmplt,
    rmapWaitReply,
    rmapRxReplyCmplt,
} RMAP_stateMachine_en;

typedef enum
{
    RMAP_notRunOperation = 0,
    RMAP_runOperation = 1,
    RMAP_compltOperation = 2,
    RMAP_abortOperation = 3
} RMAP_operStatus_en;

typedef struct
{
    uint8_t spwPort; // 0 or 1
    RMAP_cmdWriteReadHeadLogAddr_t cmdWRheadLogAddr;
    RMAP_WriteReplyHead_t writeReplyHead;
    RMAP_ReadReplyHead_t readReplyHead;
    RMAP_cmdWriteReadHeadPathAddr_t cmdWRheadPathAddr;
    uint32_t timeTxCmd;
    uint32_t timeTxReply;
    uint32_t timeWaitRxReply;
    uint32_t rxReplyTimeout;
    uint32_t txTimeout;
    uint32_t timerPeriod;
    volatile RMAP_stateMachine_en stateMachine;
    volatile RMAP_errFlag_en errorCodeOperation;
    volatile RMAP_operStatus_en operationStatus;
} RMAP_protocol_t;

extern RMAP_protocol_t RMAPcontrol;

#pragma pack(push,1)
typedef struct
{
    uint8_t reserve[11];
    uint8_t repeatWriteCmd;
    uint8_t repeatWriteCmdMax;
    uint8_t targetLogAddr;
    uint8_t initiatorLogAddr;
    uint8_t targetSpwAddr[2];
    uint16_t transactID;
    RMAP_instruct_u instruction;
    uint8_t errorLastOperation; //RMAP_errFlag_en
    uint16_t error[COUNT_ERR_MAX_ELEMENTS_RMAP];
} RMAP_HK_t;
#pragma pack(pop)

extern RMAP_HK_t RMAP_HK;


typedef enum
{
    UART_errNone = 0,					///< 00
    UART_errRxUnknownData,				///< 01
    UART_errTxDMAfail,					///< 02
    UART_errTxTimeout,					///< 03
    UART_errOverflowFIFO,				///< 04
    UART_errLineBreak,					///< 05
    RMAP_errParityErr,					///< 06
    RMAP_errFrameErr,					///< 07
    RMAP_errRxTimeout,					///< 08
    COUNT_ERR_MAX_ELEMENTS 				= 20,
} UART_operErrCode_enum;

#pragma pack(push,1)
typedef struct
{
    uint8_t reserve[9];
    volatile uint8_t errorLastOperation; // UART_operErrCode_enum
    volatile uint16_t error[COUNT_ERR_MAX_ELEMENTS];
} UART_HK_t;
#pragma pack(pop)

extern UART_HK_t UART_HK;


#pragma pack(push,1)
typedef struct
{
    uint8_t stack:			1;
    uint8_t ram:			1;
    uint8_t pFunc_start:	1;
    uint8_t pFunc_end:		1;
    uint8_t reserve:		4;
} Guard_memory_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef union
{
    uint8_t all;
    Guard_memory_t bit;
} Guard_memory_u;
#pragma pack(pop)


typedef enum
{
    operationBeginOk 		= 0,
    operationOk				= 1,
    unknownOperType			= 2,
    wrongNANDaddr			= 3,
    wrongInputParameters	= 4,
    DMAchannelBusy			= 5,
    RDYnotSet0				= 6,
    RDYnotSetBackTo1		= 7,
    DMAnotRun				= 8,
    DMAnotIRQ				= 9,
    DMAtimeout				= 10,
    DMAruntimeError			= 11,
    readStatusFail			= 12,
    featuresNotSet			= 13,
    repeatOperationFail		= 14,
    COUNT_ERR_NAND_FLASH_MAX = 20,
} NAND_operErrCode_enum;

#pragma pack(push,1)
typedef struct
{
    uint16_t column;
    uint8_t page;
    uint16_t block;
    uint8_t LUN;
    uint8_t numTarget[2];	///< numTarg[0] - target Flash number with main data,
                            ///< numTarg[1] - target Flash number with control data
} NAND_address_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint8_t reserve[11];
    NAND_address_t addr;
    volatile uint32_t timeEraseBlock;
    volatile uint16_t timeReadPage;
    volatile uint16_t timeProgPage;
    volatile uint8_t lastOperation; // NAND_operType_enum
    volatile uint8_t lastOperationStatus; // NAND_operStatus_enum
    volatile uint8_t errorLastOperation; // NAND_operErrCode_enum
    uint16_t error[COUNT_ERR_NAND_FLASH_MAX];
} NAND_HK_t;
#pragma pack(pop)

extern NAND_HK_t NAND_HK;

typedef enum
{
    system_ok = 0,
    memory_overflow = 1,
    MAX_SYSTEM_ERROR = 255,
} system_error_enum;

#define HK_VERSION 1
#define HK_MAGIC 0x484B3031u  // "HK01"

#pragma pack(push,1)
typedef struct
{
    uint32_t magic;
    uint64_t version;
    uint16_t size;
    uint32_t counter;

    uint8_t system_error;
    Guard_memory_u guard;

    UART_HK_t uart;
    RMAP_HK_t rmap;
    NAND_HK_t nand;

    uint8_t crc;

} Snapshot_HK_t;
#pragma pack(pop)



#endif // DECLARATIONS_H
