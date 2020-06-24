TEMPLATE = subdirs
CONFIG += ordered

# in this way we can specify the makefile filename, otherwise the filename would be
# Makefile.ApplInterface called directly with qmake -r
# Makefile.ApplClass.ApplInterface called though  Makefile with target subdir
SUBDIRS += ApplClass
ApplClass.file = ../idal/protocols2/ApplClass.pro
ApplClass.makefile = Makefile.ApplClass

!android {
SUBDIRS += EmbedXml
EmbedXml.file = ../idal/protocols2/EmbedXml/EmbedXml.pro
EmbedXml.makefile = Makefile.EmbedXml
}

SUBDIRS += Protocols2
Protocols2.file = ../idal/protocols2/Protocols2.pro
Protocols2.makefile = Makefile.Protocols2

SUBDIRS += icu
icu.file = ../idal/icu/icu.pro
icu.makefile = Makefile.icu

SUBDIRS += HMIIdalInterface
HMIIdalInterface.file = ../qthmi/QTHmi/HMIIdalInterface/HMIIdalInterface.pro
HMIIdalInterface.makefile = Makefile.HMIIdalInterface

SUBDIRS += HMIAbout
HMIAbout.file = ../qthmi/QTHmi/HMIAbout/HMIAbout.pro
HMIAbout.makefile = Makefile.HMIAbout

SUBDIRS += HMIViewerApp
HMIViewerApp.file = ../qthmi/QTHmi/HMIViewerApp.pro
HMIViewerApp.makefile = Makefile.hmiviewerapp

#greaterThan(QT_MAJOR_VERSION, 4) {
    SUBDIRS += HMIClient
    HMIClient.file = ../qthmi/QTHmi/HMIClient.pro
    HMIClient.makefile = Makefile.hmiclient
#}

!android {
    SUBDIRS += HMIUpdate
    HMIUpdate.file = ../qthmi/QTHmi/HMIUpdateLnx/HMIUpdate.pro
    HMIUpdate.makefile = Makefile.HMIUpdate


#    SUBDIRS += TimeLogTool
#    TimeLogTool.file = ../qthmi/QTHmi/TimeLogTool/TimeLogTool.pro
#    TimeLogTool.makefile = Makefile.TimeLogTool

    SUBDIRS += BackUp
    BackUp.file = ../qthmi/QTHmi/BackUp/BackUp.pro
    BackUp.makefile = Makefile.BackUp

    SUBDIRS += FTPUpdater
    FTPUpdater.file = ../qthmi/QTHmi/FTPUpdater/FTPUpdater.pro
    FTPUpdater.makefile = Makefile.FTPUpdater

    !greaterThan(QT_MAJOR_VERSION, 4) {
        SUBDIRS += HTTPUpdateUtility
        HTTPUpdateUtility.file = ../qthmi/QTHmi/HTTPUpdateUtility/HTTPUpdateUtility.pro
        HTTPUpdateUtility.makefile = Makefile.HTTPUpdateUtility
    }

    SUBDIRS += WatchdogDialog
    WatchdogDialog.file = ../qthmi/QTHmi/WatchdogDialog/WatchdogDialog.pro
    WatchdogDialog.makefile = Makefile.WatchdogDialog

}

OTHER_FILES += TODO.txt \
    android/res/values-id/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-et/strings.xml \
    android/res/values/strings.xml \
    android/res/values-de/strings.xml \
    android/res/layout/splash.xml \
    android/res/values-ja/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-nb/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/values-es/strings.xml \
    android/version.xml \
    android/src/org/kde/necessitas/origo/QtApplication.java \
    android/src/org/kde/necessitas/origo/QtActivity.java \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/res/drawable-hdpi/icon.png \
    android/res/drawable-mdpi/icon.png \
    android/res/values/libs.xml \
    android/res/drawable-ldpi/icon.png \
    android/res/drawable/logo.png \
    android/res/drawable/icon.png \
    android/AndroidManifest.xml
