# Shared library without any Qt functionality
TEMPLATE = lib
QT -= gui core

CONFIG += warn_on plugin debug
CONFIG -= thread exceptions qt rtti release

VERSION = 1.0.0

INCLUDEPATH += ../../SDK/CHeaders/XPLM
INCLUDEPATH += ../../SDK/CHeaders/Wrappers
INCLUDEPATH += ../../SDK/CHeaders/Widgets
INCLUDEPATH += ..
VPATH = ..

# Defined to use X-Plane SDK 2.0 capabilities - no backward compatibility before 9.0
DEFINES += XPLM200

win32 {
    DEFINES += APL=0 IBM=1 LIN=0
    # SOURCES += speech_generic.c
    SOURCES += speech_sapi.cpp
    LIBS += -L../../SDK/Libraries/Win
    LIBS += -lXPLM -lXPWidgets
    TARGET = win.xpl
    LIBS += "-LD:\\Program Files\\Microsoft SDKs\\Windows\\v7.1\\Lib"
    LIBS +=  "-lsapi"
    LIBS +=  "-lole32"
    INCLUDEPATH += D:/gnu/include
    INCLUDEPATH += "D:\\Program Files\\Microsoft SDKs\\Windows\\v7.1\\Include"
    INCLUDEPATH += .
}

unix:!macx {
    DEFINES += APL=0 IBM=0 LIN=1
    TARGET = lin.xpl
    # WARNING! This requires the latest version of the X-SDK !!!!
    QMAKE_CXXFLAGS += -fvisibility=hidden -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2
    QMAKE_CFLAGS += -fvisibility=hidden -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2
    SOURCES += speech_sd.c
    LIBS += -ldl -Wl,--version-script -Wl,../Xchecklist.sym
}

macx {
    DEFINES += APL=1 IBM=0 LIN=0
    TARGET = mac.xpl
    QMAKE_CXXFLAGS += -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2
    QMAKE_CFLAGS += -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2
    QMAKE_LFLAGS += -flat_namespace -undefined suppress
    
    SOURCES += messages.c whisperer.c speech_mac.c
    HEADERS += messages.h whisperer.h
    # OBJECTIVE_SOURCES += speech_mac.m
    # Build for multiple architectures.
    # The following line is only needed to build universal on PPC architectures.
    # QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
    # The following line defines for which architectures we build.
    CONFIG += x86_64 x86
    LIBS += -ldl -exported_symbols_list ../Xchecklist.sym_mac
}

HEADERS += interface.h \
           chkl_parser.h \
           speech.h \
    stdbool.h \
    utils.h


SOURCES += Xchecklist.cpp \
           interface.cpp \
           parser.cpp \
           utils.cpp

LEXSOURCES += chkl.l
YACCSOURCES += chkl.y

