TEMPLATE = lib
CONFIG += plugin
QT += core gui widgets gui-private
TARGET = InputPanelContextPlugin

target.path = $$[QT_INSTALL_PLUGINS]/platforminputcontexts

INCLUDEPATH += .
DEFINES += INPUTPANELCONTEXTPLUGINSHARED_LIBRARY

INSTALLS += target

include(../../../../linuxbuild/lin-common.pri)

# Input
HEADERS += InputPanelContext.h InputPanelProxy.h
SOURCES += InputPanelContext.cpp InputPanelProxy.cpp
RESOURCES +=
OTHER_FILES += InputPanelContextPlugin.json
