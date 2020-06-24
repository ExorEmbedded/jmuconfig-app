greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets webkitwidgets webkit
} else {
    QT      +=  webkit network gui
}

CONFIG += console
HEADERS =   mainwindow.h \
    inputpanel/myinputpanelcontext.h \
    inputpanel/myinputpanel.h \
    flickcharm.h
SOURCES =   main.cpp \
            mainwindow.cpp \
    inputpanel/myinputpanelcontext.cpp \
    inputpanel/myinputpanel.cpp \
    flickcharm.cpp
RESOURCES = jquery.qrc \
    icons.qrc

INCLUDEPATH+=inputpanel

FORMS += \
    inputpanel/myinputpanelform.ui \
    form.ui \
    mainwindow.ui

