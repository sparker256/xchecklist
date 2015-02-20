# Shared library without any Qt functionality
TEMPLATE = lib
QT -= gui core

CONFIG += warn_on plugin release
CONFIG -= thread exceptions qt rtti debug

VERSION = 1.0.0

INCLUDEPATH += ../../SDK/CHeaders/XPLM
INCLUDEPATH += ../../SDK/CHeaders/Wrappers
INCLUDEPATH += ../../SDK/CHeaders/Widgets
INCLUDEPATH += ..
VPATH = ..

# Defined to use X-Plane SDK 2.0 capabilities - no backward compatibility before 9.0
DEFINES += XPLM200

win32{
    message(win32)
    CONFIG += dll
    DEFINES += APL=0 IBM=1 LIN=0
    # SOURCES += speech_generic.c
    SOURCES += speech_sapi.cpp
    LIBS += -L../../SDK/Libraries/Win
    TARGET = win.xpl
    INCLUDEPATH += .
    LIBS +=  "-lole32" "-luuid" "-lsapi"
}

win32:isEmpty(CROSS_COMPILE){
    message(win32nocross)
    LIBS += -lXPLM -lXPWidgets
    LIBS += "-LD:\\Program Files\\Microsoft SDKs\\Windows\\v7.1\\Lib"
    INCLUDEPATH += D:/gnu/include
    INCLUDEPATH += "D:\\Program Files\\Microsoft SDKs\\Windows\\v7.1\\Include"
}

win32:!isEmpty(CROSS_COMPILE){
    message(win32cross)
    QMAKE_YACC = yacc
    QMAKE_YACCFLAGS_MANGLE  += -p $base -b $base
    QMAKE_YACC_HEADER       = $base.tab.h
    QMAKE_YACC_SOURCE       = $base.tab.c
    QMAKE_DEL_FILE          = rm -f
    INCLUDEPATH += "../../WinSDK/Include"
    LIBS += -static-libstdc++ -static-libgcc -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic
}

win32:contains(CROSS_COMPILE, x86_64-w64-mingw32-){
    message(win32cross64)
    LIBS += -L"../../WinSDK/Lib/x64"
    LIBS += -lXPLM_64 -lXPWidgets_64
}

win32:contains(CROSS_COMPILE, i686-w64-mingw32-){
    message(win32cross32)
    LIBS += -L"../../WinSDK/Lib"
    LIBS += -lXPLM -lXPWidgets
    DEFINES += __MIDL_user_allocate_free_DEFINED__
}

unix:!macx {
    DEFINES += APL=0 IBM=0 LIN=1
    TARGET = lin.xpl
    # WARNING! This requires the latest version of the X-SDK !!!!
    QMAKE_CXXFLAGS += -fvisibility=hidden -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2
    QMAKE_CFLAGS += -fvisibility=hidden -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2
    SOURCES += messages.c whisperer.c speech_mac.c
    HEADERS += messages.h whisperer.h
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
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5
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

