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
DEFINES += ARM

TRANSLATIONS += trans/en.ts

if(contains(DEFINES,ARM)) {
    message( "Configuring for arm build..." )
} else {
    message( "Configuring for pc build..." )
}

SOURCES += \
    Gui/MainMenu/Menu0/menu0.cpp \
    Gui/MainMenu/Menu1/menu1.cpp \
    Gui/MainMenu/Menu4/menu4.cpp \
    Gui/MainMenu/Menu5/menu5.cpp \
    Gui/MainMenu/Menu6/menu6.cpp \
    Gui/MainMenu/StatusBar/statusbar.cpp \
    Gui/MainMenu/mainmenu.cpp \
    Gui/MainMenu/Menu4/HFCAmplitude/hfcamplitude.cpp \
    Gui/MainMenu/Menu4/HFCAtlas/hfcatlas.cpp \
    Gui/MainMenu/Menu4/HFCAmplitude/hfcbarchart.cpp \
    Gui/MainMenu/Menu6/SystemInfo/systeminfo.cpp \
    Gui/MainMenu/Menu6/DebugSet/debugset.cpp \
    IO/Data/fifodata.cpp \
    IO/Key/keydetect.cpp \
    IO/Modbus/modbus.cpp \
    IO/SqlCfg/sqlcfg.cpp \
    main.cpp \
    Gui/mainwindow.cpp \
    Gui/MainMenu/Menu6/Option/options.cpp \
    Gui/MainMenu/Menu6/FactoryReset/factoryreset.cpp \
    IO/Modbus/gpio_oper.c \
    IO/Modbus/uart_oper.c \
    IO/Data/recwave.cpp \
    Gui/MainMenu/StatusBar/battery.cpp \
    Gui/MainMenu/Menu0/tevwidget.cpp \
    Gui/MainMenu/Menu0/barchart.cpp \
    IO/Data/filetools.cpp \
    Gui/MainMenu/Menu6/DebugSet/recwaveform.cpp \
    Gui/MainMenu/Menu2/menu2.cpp \
    Gui/MainMenu/Menu3/menu3.cpp \
    Gui/MainMenu/Menu2/faultlocation.cpp \
    Gui/MainMenu/Menu3/aawidget.cpp \
    Gui/MainMenu/Menu6/RecWaveManage/recwavemanage.cpp \
    Gui/MainMenu/Menu5/rfctwidget.cpp \
    IO/Data/logtools.cpp


HEADERS  += \
    Gui/MainMenu/Menu0/menu0.h \
    Gui/MainMenu/Menu1/menu1.h \
    Gui/MainMenu/Menu4/menu4.h \
    Gui/MainMenu/Menu5/menu5.h \
    Gui/MainMenu/Menu6/menu6.h \
    Gui/MainMenu/StatusBar/statusbar.h \
    Gui/MainMenu/mainmenu.h \
    Gui/MainMenu/Menu4/HFCAmplitude/hfcamplitude.h \
    Gui/MainMenu/Menu4/HFCAtlas/hfcatlas.h \
    Gui/MainMenu/Menu4/HFCAmplitude/hfcbarchart.h \
    Gui/MainMenu/Menu6/SystemInfo/systeminfo.h \
    Gui/MainMenu/Menu6/DebugSet/debugset.h \
    IO/Data/fifodata.h \
    IO/Data/zynq.h \
    IO/Key/key.h \
    IO/Key/keydetect.h \
    IO/Modbus/modbus.h \
    IO/SqlCfg/sqlcfg.h \
    Gui/mainwindow.h \
    Gui/MainMenu/Menu6/Option/options.h \
    Gui/MainMenu/Menu6/FactoryReset/factoryreset.h \
    IO/Modbus/gpio_oper.h \
    IO/Modbus/uart_oper.h \
    IO/Data/data.h \
    IO/Data/recwave.h \
    Gui/MainMenu/StatusBar/battery.h \
    Gui/MainMenu/Menu0/tevwidget.h \
    Gui/MainMenu/Menu0/barchart.h \
    IO/Data/filetools.h \
    Gui/MainMenu/Menu6/DebugSet/recwaveform.h \
    Gui/MainMenu/Menu2/menu2.h \
    Gui/MainMenu/Menu3/menu3.h \
    Gui/MainMenu/Menu2/faultlocation.h \
    Gui/MainMenu/Menu3/aawidget.h \
    Gui/MainMenu/Menu6/RecWaveManage/recwavemanage.h \
    Gui/MainMenu/Menu5/rfctwidget.h \
    IO/Data/logtools.h


RESOURCES += \
    resource/resource.qrc

################################################################################
INCLUDEPATH += \
    /usr/local/qwt-6.1.3/include \
    /usr/local/sqlite3/include

if(contains(DEFINES,ARM)) {
# For ARM
LIBS += \
    -L"/usr/local/qwt-6.1.3/lib" -lqwt \
    -L"/usr/local/sqlite3/lib" -lsqlite3
} else {
# For PC
LIBS += \
    -L"/usr/local/qwt-6.1.3-pc/lib" -lqwt \
    -L"/usr/local/sqlite3-pc/lib" -lsqlite3
}

#INCLUDEPATH += \
#    /usr/local/qwtplot3d/include

#LIBS += \
#    -L/usr/local/qwtplot3d/lib -lqwtplot3d -lGLU

DISTFILES += \
    BOOT.bin \
    PDTEV操作指南_V1.1.docx

FORMS += \
    Gui/MainMenu/Menu6/Option/optionui.ui \
    Gui/MainMenu/Menu6/SystemInfo/systeminfo.ui \
    Gui/MainMenu/Menu6/DebugSet/debugui.ui \
    Gui/MainMenu/Menu0/amplitude1.ui \
    Gui/MainMenu/Menu2/faultlocation.ui \
    Gui/MainMenu/Menu3/aawidget.ui \
    Gui/MainMenu/Menu6/RecWaveManage/voiceplayer.ui \
    Gui/MainMenu/Menu5/rfctwidget.ui
