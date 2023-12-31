include(../tests.pri)

QT += core \
    sql \
    testlib \
    xml \
    network \
    dbus

QT -= gui

LIBS += \
    -lsignon-extension \
    -lsignon-qt5

QMAKE_LIBDIR += \
    $${TOP_BUILD_DIR}/lib/signond/SignOn \
    $${TOP_BUILD_DIR}/lib/SignOn
QMAKE_RPATHDIR = $${QMAKE_LIBDIR}

SIGNOND_SRC = $${TOP_SRC_DIR}/src/signond

DEFINES += SIGNOND_TRACE
DEFINES += SIGNON_PLUGIN_TRACE

INCLUDEPATH += . \
    $$TOP_SRC_DIR/lib/plugins \
    $$TOP_SRC_DIR/lib/signond \
    $$TOP_SRC_DIR/src/signond \
    $${TOP_SRC_DIR}/lib/plugins/signon-plugins-common \
    $${TOP_SRC_DIR}/lib/plugins/signon-plugins-common/SignOn \
    $${TOP_SRC_DIR}/lib \

check.depends = $$TARGET
check.commands = "SSO_PLUGINS_DIR=$${TOP_BUILD_DIR}/src/plugins/test SSO_EXTENSIONS_DIR=$${TOP_BUILD_DIR}/non-existing-dir $$RUN_WITH_SIGNOND ./$$TARGET"
