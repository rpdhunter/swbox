#-------------------------------------------------
#
# Project created by QtCreator 2016-04-06T17:14:13
#
#-------------------------------------------------

QT       += core gui widgets network

QT += serialport sql
#QT += opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = swbox
TEMPLATE = app

#DEFINES += QT_NO_DEBUG_OUTPUT
#DEFINES += PRINTSCREEN     #截屏
#DEFINES += AMG          #OEM
#DEFINES += OHV          #OEM

TRANSLATIONS += trans/en.ts

#if(contains(DEFINES,ARM)) {
#    message( "Configuring for arm build..." )
#} else {
#    message( "Configuring for pc build..." )
#}

SOURCES += \
    IO/Data/fifodata.cpp \
    IO/Data/recwave.cpp \
    IO/SqlCfg/sqlcfg.cpp \
    IO/Key/keydetect.cpp \
    IO/Modbus/modbus.cpp \
    IO/Modbus/gpio_oper.c \
    IO/Modbus/uart_oper.c \
    IO/rdb/thread.c \
    IO/rdb/rdb.c \
    IO/rdb/ipcs.c \
    main.cpp \
    Gui/Common/common.cpp \
    Gui/Common/barchart.cpp \
    Gui/Function/tevwidget.cpp \
    Gui/Function/hfctwidget.cpp \
    Gui/Function/faultlocation.cpp \
    Gui/Function/aawidget.cpp \
    IO/Other/battery.cpp \
    Gui/Options/systeminfo.cpp \
    Gui/Options/recwavemanage.cpp \
    Gui/Options/options.cpp \
    Gui/Options/factoryreset.cpp \
    Gui/Options/debugset.cpp \
    Gui/Common/recwaveform.cpp \
    Gui/Function/aewidget.cpp \
    Gui/mainwindow.cpp \
    IO/Other/logtools.cpp \
    IO/Other/filetools.cpp \
    IO/Data/reccontrol.cpp \
    IO/Data/fifocontrol.cpp \
    IO/Other/wifi.cpp \
    Gui/Common/wifitools.cpp


HEADERS  += \
    IO/Data/fifodata.h \
    IO/Data/zynq.h \
    IO/Data/data.h \
    IO/Data/recwave.h \
    IO/Key/key.h \
    IO/Key/keydetect.h \
    IO/Modbus/modbus.h \
    IO/Modbus/gpio_oper.h \
    IO/Modbus/uart_oper.h \
    IO/SqlCfg/sqlcfg.h \    
    IO/rdb/thread.h \
    IO/rdb/rdb.h \
    IO/rdb/point_table.h \
    IO/rdb/ipcs.h \
    IO/rdb/data_types.h \
    IO/rdb/comm_head.h \
    Gui/Common/common.h \
    Gui/Common/barchart.h \
    Gui/Function/tevwidget.h \
    Gui/Function/hfctwidget.h \
    Gui/Function/faultlocation.h \
    Gui/Function/aawidget.h \
    IO/Other/battery.h \
    Gui/Options/systeminfo.h \
    Gui/Options/recwavemanage.h \
    Gui/Options/options.h \
    Gui/Options/factoryreset.h \
    Gui/Options/debugset.h \
    Gui/Common/recwaveform.h \
    Gui/Function/aewidget.h \
    Gui/mainwindow.h \
    IO/Other/logtools.h \
    IO/Other/filetools.h \
    IO/Data/reccontrol.h \
    IO/Data/fifocontrol.h \
    IO/Other/wifi.h \
    Gui/Common/wifitools.h


RESOURCES += \
    resource/resource.qrc

################################################################################
INCLUDEPATH += \
    /usr/local/qwt-6.1.3/include \
    /usr/local/sqlite3/include

LIBS += \
    -L"/usr/local/qwt-6.1.3/lib" -lqwt \
    -L"/usr/local/sqlite3/lib" -lsqlite3

unix:!macx: LIBS += -L$$PWD/../../../../pub/toolchain/arm-xilinx-linux-gnueabi-4.9.2/arm-xilinx-linux-gnueabi/libc/usr/lib/ -lpthread

INCLUDEPATH += $$PWD/../../../../pub/toolchain/arm-xilinx-linux-gnueabi-4.9.2/arm-xilinx-linux-gnueabi/libc/usr/include
DEPENDPATH += $$PWD/../../../../pub/toolchain/arm-xilinx-linux-gnueabi-4.9.2/arm-xilinx-linux-gnueabi/libc/usr/include

unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../pub/toolchain/arm-xilinx-linux-gnueabi-4.9.2/arm-xilinx-linux-gnueabi/libc/usr/lib/libpthread.a

#INCLUDEPATH += \
#    /usr/local/qwtplot3d/include

#LIBS += \
#    -L/usr/local/qwtplot3d/lib -lqwtplot3d -lGLU

DISTFILES += \
    BOOT.bin \
    trans/en.ts

FORMS += \    
    Gui/Function/tevwidget.ui \
    Gui/Function/hfctwidget.ui \
    Gui/Function/faultlocation.ui \
    Gui/Function/aawidget.ui \
    Gui/Options/voiceplayer.ui \
    Gui/Options/systeminfo.ui \
    Gui/Options/optionui.ui \
    Gui/Options/debugui.ui \
    Gui/Function/aewidget.ui \
    Gui/mainwindow.ui

