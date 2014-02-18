# Shared library without any Qt functionality
TEMPLATE = app
QT -= gui core

CONFIG += warn_on plugin debug
CONFIG -= thread exceptions qt rtti release


VERSION = 1.0.0

INCLUDEPATH += ../../SDK/CHeaders/XPLM
INCLUDEPATH += ../../SDK/CHeaders/Wrappers
INCLUDEPATH += ../../SDK/CHeaders/Widgets
INCLUDEPATH += ..

# Defined to use X-Plane SDK 2.0 capabilities - no backward compatibility before 9.0
DEFINES += XPLM200 CHECKER

win32 {
    DEFINES += APL=0 IBM=1 LIN=0
    LIBS += -L../../SDK/Libraries/Win
    # LIBS += -lXPLM -lXPWidgets
    CONFIG += console
}

win32:isEmpty(CROSS_COMPILE){
    LIBS += -LD:/Qt/Qt5.1.0_32/5.1.0/msvc2010_opengl/lib
    INCLUDEPATH += D:/gnuwin32/include
#    LIBS += -lqtmain
}

win32:!isEmpty(CROSS_COMPILE){
    QMAKE_YACC = yacc
    QMAKE_YACCFLAGS_MANGLE  += -p $base -b $base
    QMAKE_YACC_HEADER       = $base.tab.h
    QMAKE_YACC_SOURCE       = $base.tab.c
    QMAKE_DEL_FILE          = rm -f
    LIBS += -static-libstdc++ -static-libgcc
}

unix:!macx {
    DEFINES += APL=0 IBM=0 LIN=1
    # WARNING! This requires the latest version of the X-SDK !!!!
    QMAKE_CXXFLAGS += -fvisibility=hidden -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2
    QMAKE_CFLAGS += -fvisibility=hidden -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2
}

macx {
    DEFINES += APL=1 IBM=0 LIN=0

    QMAKE_CXXFLAGS += -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2 -fpermissive
    QMAKE_CFLAGS += -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2
    QMAKE_LFLAGS += -flat_namespace -undefined suppress
    CONFIG -= app_bundle

    CONFIG += x86_64 x86

}

HEADERS += ../interface.h \
           ../chkl_parser.h \
           ../speech.h \
    ../stdbool.h \
    ../utils.h


SOURCES += main.c \
           ../interface.cpp \
           ../parser.cpp \
           ../utils.cpp

LEXSOURCES += ../chkl.l
YACCSOURCES += ../chkl.y

