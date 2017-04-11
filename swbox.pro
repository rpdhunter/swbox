#-------------------------------------------------
#
# Project created by QtCreator 2016-04-06T17:14:13
#
#-------------------------------------------------

QT       += core gui widgets network

QT += serialport

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
    Gui/MainMenu/Menu0/Amplitude/amplitude.cpp \
    Gui/MainMenu/Menu0/Pulse/pulse.cpp \
    Gui/MainMenu/Menu0/menu0.cpp \
    Gui/MainMenu/Menu1/AAUltrasonic/aaultrasonic.cpp \
    Gui/MainMenu/Menu1/menu1.cpp \
    Gui/MainMenu/Menu2/PhaseSpectra/phasespectra.cpp \
    Gui/MainMenu/Menu2/PulseSpectra/pulsespectra.cpp \
    Gui/MainMenu/Menu2/WaveSpectra/wavespectra.cpp \
    Gui/MainMenu/Menu2/menu2.cpp \
    Gui/MainMenu/Menu3/menu3.cpp \
    Gui/MainMenu/Menu4/menu4.cpp \
    Gui/MainMenu/Menu5/menu5.cpp \
    Gui/MainMenu/Menu6/menu6.cpp \
    Gui/MainMenu/StatusBar/statusbar.cpp \
    Gui/MainMenu/mainmenu.cpp \
    Gui/MainMenu/Menu3/HFAmplitude/hfamplitude.cpp \
    Gui/MainMenu/Menu3/HFAtlas/hfatlas.cpp \
    Gui/MainMenu/Menu3/HFPrps/hfprps.cpp \
    Gui/MainMenu/Menu5/BackRecd/backrecd.cpp \
    Gui/MainMenu/Menu5/EnvRecd/envrecd.cpp \
    Gui/MainMenu/Menu5/LoadData/loaddata.cpp \
    Gui/MainMenu/Menu5/TestPos/testpos.cpp \
    Gui/MainMenu/Menu4/HFCAmplitude/hfcamplitude.cpp \
    Gui/MainMenu/Menu4/HFCAtlas/hfcatlas.cpp \
    Gui/MainMenu/Menu0/Amplitude/barchart.cpp \
    Gui/MainMenu/Menu3/HFAmplitude/hfbarchart.cpp \
    Gui/MainMenu/Menu4/HFCAmplitude/hfcbarchart.cpp \
    Gui/MainMenu/Menu6/SystemInfo/systeminfo.cpp \
    Gui/MainMenu/Menu6/DebugSet/debugset.cpp \
    Gui/MainMenu/Menu1/AAUltrasonic/aaultrasonic2.cpp \
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
    Gui/MainMenu/StatusBar/battery.cpp


HEADERS  += \
    Gui/MainMenu/Menu0/Amplitude/amplitude.h \
    Gui/MainMenu/Menu0/Pulse/pulse.h \
    Gui/MainMenu/Menu0/menu0.h \
    Gui/MainMenu/Menu1/AAUltrasonic/aaultrasonic.h \
    Gui/MainMenu/Menu1/menu1.h \
    Gui/MainMenu/Menu2/PhaseSpectra/phasespectra.h \
    Gui/MainMenu/Menu2/PulseSpectra/pulsespectra.h \
    Gui/MainMenu/Menu2/WaveSpectra/wavespectra.h \
    Gui/MainMenu/Menu2/menu2.h \
    Gui/MainMenu/Menu3/menu3.h \
    Gui/MainMenu/Menu4/menu4.h \
    Gui/MainMenu/Menu5/menu5.h \
    Gui/MainMenu/Menu6/menu6.h \
    Gui/MainMenu/StatusBar/statusbar.h \
    Gui/MainMenu/mainmenu.h \
    Gui/MainMenu/Menu3/HFAmplitude/hfamplitude.h \
    Gui/MainMenu/Menu3/HFAtlas/hfatlas.h \
    Gui/MainMenu/Menu3/HFPrps/hfprps.h \
    Gui/MainMenu/Menu5/BackRecd/backrecd.h \
    Gui/MainMenu/Menu5/EnvRecd/envrecd.h \
    Gui/MainMenu/Menu5/LoadData/loaddata.h \
    Gui/MainMenu/Menu5/TestPos/testpos.h \
    Gui/MainMenu/Menu4/HFCAmplitude/hfcamplitude.h \
    Gui/MainMenu/Menu4/HFCAtlas/hfcatlas.h \
    Gui/MainMenu/Menu0/Amplitude/barchart.h \
    Gui/MainMenu/Menu3/HFAmplitude/hfbarchart.h \
    Gui/MainMenu/Menu4/HFCAmplitude/hfcbarchart.h \
    Gui/MainMenu/Menu6/SystemInfo/systeminfo.h \
    Gui/MainMenu/Menu6/DebugSet/debugset.h \
    Gui/MainMenu/Menu1/AAUltrasonic/aaultrasonic2.h \
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
    Gui/MainMenu/StatusBar/battery.h


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

DISTFILES +=

FORMS += \
    Gui/MainMenu/Menu6/Option/optionui.ui \
    Gui/MainMenu/Menu6/SystemInfo/systeminfo.ui \
    Gui/MainMenu/Menu6/DebugSet/debugui.ui
