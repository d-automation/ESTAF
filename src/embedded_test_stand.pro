include(gtest_dependency.pri)
QT -= gui
QT += serialport

CONFIG += console
CONFIG -= app_bundle
CONFIG += thread

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += \
    $$PWD/core \
    $$PWD/tests \
    $$PWD/configs

SOURCES += \
    core/cli_parser.cpp \
    core/crc8.cpp \
    core/html_report.cpp \
    core/json_processor.cpp \
    core/logger.cpp \
    core/loggingCategories.cpp \
    core/test_loader.cpp \
    core/test_options.cpp \
    main.cpp \
    core/uart.cpp \
    core/protocol_handler.cpp \
    tests/test_runner.cpp \
    tests/uart_fixture.cpp

HEADERS += \
    core/cli_parser.h \
    core/crc8.h \
    core/declarations.h \
    core/exceptions_handle.h \
    core/html_report.h \
    core/json_processor.h \
    core/logger.h \
    core/loggingCategories.h \
    core/test_loader.h \
    core/test_options.h \
    core/test_stats.h \
    core/uart.h \
    core/uart_port.h \
    core/protocol_handler.h \
    tests/uart_fixture.h

DISTFILES += \
    ../config.json \
    ../configs/test_cases/test_UART_cmdSetDataInterface.json \
    ../configs/test_cases/test_abort.json \
    ../configs/test_cases/test_endurance_50_60_marchFTE_5.json \
    ../configs/test_cases/test_erase_all.json \
    ../configs/test_cases/test_get_badBlockMap_fromNAND.json \
    ../configs/test_cases/test_randomDataTest_100_110.json \
    ../configs/test_cases/test_readSnapshot_HK.json \
    ../configs/test_cases/test_read_NANDctrlOper.json \
    ../configs/test_cases/test_set_micron_MT29F16G08AJADAWP.json \
    ../configs/test_cases/test_start_March_FTE_all_targ0_repeat2.json \
    ../configs/test_cases/test_start_March_FTE_by_blocks.json \
    ../configs/test_cases/test_start_retention_rand_25_bin.json \
    ../configs/test_plan.json
