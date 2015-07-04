# Shared library without any Qt functionality
TEMPLATE = app
QT -= gui core

CONFIG += warn_on plugin debug
CONFIG -= thread exceptions qt rtti release app_bundle

VERSION = 1.0.0

INCLUDEPATH += ../../SDK/CHeaders/XPLM
INCLUDEPATH += ../../SDK/CHeaders/Wrappers
INCLUDEPATH += ../../SDK/CHeaders/Widgets
INCLUDEPATH += ..
VPATH += ..
    
SOURCES += messages.c
HEADERS += messages.h

# Defined to use X-Plane SDK 2.0 capabilities - no backward compatibility before 9.0
DEFINES += XPLM200


unix:!macx {
    DEFINES += APL=0 IBM=0 LIN=1
    # WARNING! This requires the latest version of the X-SDK !!!!
    QMAKE_CXXFLAGS += -fvisibility=hidden -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2
    QMAKE_CFLAGS += -fvisibility=hidden -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2
    SOURCES += simon.c speech_sd.c
    HEADERS += speech.h
    LIBS += -ldl -pthread
    TARGET = simon_lin
}

macx {
    QMAKE_CXXFLAGS += -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2
    QMAKE_CFLAGS += -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2
    QMAKE_LFLAGS += -flat_namespace -undefined suppress
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5
    OBJECTIVE_SOURCES += simon.m
    # Build for multiple architectures.
    # The following line is only needed to build universal on PPC architectures.
    # QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
    # The following line defines for which architectures we build.
    CONFIG += x86_64 x86
    LIBS += -pthread -framework AppKit
    TARGET = simon_mac
}

