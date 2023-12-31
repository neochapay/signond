include( ../../common-project-config.pri )
include( ../../common-vars.pri )
TEMPLATE = app
TARGET = signonpluginprocess
QT += core network
QT -= gui

HEADERS += \
    debug.h \
    remotepluginprocess.h

SOURCES += \
    debug.cpp \
    main.cpp \
    remotepluginprocess.cpp

INCLUDEPATH += . \
               $$TOP_SRC_DIR/src \
               $$TOP_SRC_DIR/src/plugins \
               $$TOP_SRC_DIR/src/signond \
               $$TOP_SRC_DIR/lib/plugins/signon-plugins-common \
               $$TOP_SRC_DIR/lib/plugins \
               $$TOP_SRC_DIR/lib

CONFIG += \
    build_all

lessThan(QT_VERSION, "5.5.0"):system($$pkgConfigExecutable() --exists libproxy-1.0) {
    DEFINES += HAVE_LIBPROXY
    PKGCONFIG += libproxy-1.0
    SOURCES += my-network-proxy-factory.cpp
}

QMAKE_LIBDIR += \
    $${TOP_BUILD_DIR}/lib/plugins/signon-plugins-common \
    $${TOP_BUILD_DIR}/lib/plugins

LIBS += \
    -lsignon-plugins-common \
    -lsignon-plugins

DEFINES += SIGNON_PLUGIN_TRACE
DEFINES += "SIGNOND_PLUGINS_DIR=$${SIGNOND_PLUGINS_DIR_QUOTED}"

include( ../../common-installs-config.pri )
