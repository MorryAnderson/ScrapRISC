#DEFINES += NO_BP

if (contains(DEFINES,NO_BP)) {
    QT -= gui
    QT -= widgets
    TARGET = ScrapHDLCompiler_nobp
} else {
    QT += gui
    QT += widgets
    SOURCES += ../src/cpp/bp_generator.cpp
    HEADERS += ../src/cpp/bp_generator.h
    TARGET = ScrapHDLCompiler
}


CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../src/cpp/compiler.cpp \
    ../src/cpp/gate.cpp \
    ../src/cpp/main.cpp \
    ../src/cpp/makefile.cpp \
    ../src/cpp/matrix.cpp \
    ../src/cpp/module.cpp \
    ../src/cpp/node.cpp \
    ../src/cpp/node_group.cpp \
    ../src/cpp/parser.cpp \
    ../src/cpp/signal.cpp \
    ../src/cpp/sub_module.cpp \
    ../src/cpp/tunnel.cpp \
    ../src/cpp/var_info.cpp \
    ../src/cpp/wire.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ../src/cpp/bnflite.h \
    ../src/cpp/compiler.h \
    ../src/cpp/gate.h \
    ../src/cpp/global_include.h \
    ../src/cpp/makefile.h \
    ../src/cpp/matrix.h \
    ../src/cpp/module.h \
    ../src/cpp/node.h \
    ../src/cpp/node_group.h \
    ../src/cpp/parser.h \
    ../src/cpp/signal.h \
    ../src/cpp/sub_module.h \
    ../src/cpp/tunnel.h \
    ../src/cpp/var_info.h \
    ../src/cpp/wire.h
