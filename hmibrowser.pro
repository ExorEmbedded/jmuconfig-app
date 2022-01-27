TARGET=WebkitBrowser


greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets webkitwidgets webkit dbus concurrent
} else {
    QT      +=  webkit network gui dbus
}

DEFINES += HAS_WEBKIT X11_INPUT_IMPL
DEFINES += EXITPATTERN_CHECK_ON_REPLY

CONFIG += console
HEADERS =   mainwindow.h \
    flickcharm.h \
    browsersettings.h \
    cookiejar.h \
    epadinterface.h \
    autoLoginManager.h \
    loginform.h
SOURCES =   main.cpp \
            mainwindow.cpp \
    flickcharm.cpp \
    browsersettings.cpp \
    cookiejar.cpp \
    epadinterface.cpp \
    autoLoginManager.cpp \
    loginform.cpp 
RESOURCES = \
    icons.qrc

lessThan(QT_MAJOR_VERSION, 5) {
HEADERS +=  InputPanel/InputPanelContext.h \
    InputPanel/InputPanelProxy.h 
SOURCES +=  InputPanel/InputPanelContext.cpp \
    InputPanel/InputPanelProxy.cpp 
}

INCLUDEPATH+=InputPanel

#FORMS += \
#    inputpanel/myinputpanelform.ui

OTHER_FILES += simpledeploypkg.sh scripts/run.sh scripts/start.sh scripts/stop.sh scripts/uninstall.sh scripts/install.sh scripts/package.info

!wayland : LIBS += -lX11

FORMS += \
    browsersettings.ui \
    loginform.ui

target.path = /usr/bin

INSTALLS += target
