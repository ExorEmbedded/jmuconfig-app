#!/bin/bash

# Scans xml files and create deploy folder
# search path format is <search folter>:<search depth>
#
# NOTE: requires xmlpatterns tool from qt

# to build the docker
# docker run --rm -ti -v $(pwd)/../:/build  -v /opt:/opt -v /var/run/docker.sock:/var/run/docker.sock  -v /home:/home -v /etc/group:/etc/group -v /etc/passwd:/etc/passwd -u $(id -u):$(id -g)  --group-add="$( getent group docker | cut -d: -f3  )" jmobile-build-env /bin/bash -c "cd /build/linuxbuild && ./simpledeploypkg_jmobile.sh pc"


cd `dirname $0`

PKG_PREFIX=jm
BUILD_SUBDIR=build

# xmlpatterns path
export PATH=$PATH:/opt/exor/Qt512-x86-exor/bin/
if ! ( which xmlpatterns &>/dev/null ); then
	echo ERROR: $0 Can not find xmlpatterns
	exit 1
fi

# load common functions
source ./simpledeploypkg_qt.sh


die() {
	exit $1
}

# copy start script
cp ./scripts/start.sh $OUTPUTFOLDER
chmod 0777 $OUTPUTFOLDER/start.sh

# copy config files
cp -a ../qthmi/Support/Images $OUTPUTFOLDER/images
cp -a ../qthmi/Support/misc/target/* $OUTPUTFOLDER
cp -a ../qthmi/Support/install/files/exor/console/* $OUTPUTFOLDER/workspace/
mkdir $OUTPUTFOLDER/templates
cp -a ../qthmi/Support/install/files/exor/UserMgmtTemplates $OUTPUTFOLDER/templates/usermgmttemplates
cp -a ../qthmi/Support/templates/KeyPadTemplates $OUTPUTFOLDER/templates/keypadtemplates

# brand config files
echo -e $cyanstart "Translate config:" $colorend
BRANDNAME="Exor"
translate() {
     [ $HMIABOUTDIR ] && [ "$HMIABOUTDIR" != "." ] && export  BRANDNAME=$HMIABOUTDIR
    which xmlpatterns
    xmlpatterns ../jbuild/xmltransform.xsl $1  -param brandName=$BRANDNAME -output $1.tmp
    mv $1.tmp $1
}

translate_files() {
    for f in `find $OUTPUTFOLDER/config -iname "*.xml"` ; do
        echo -e $graystart "translate $f"  $colorend
        translate $f ;
    done
    for f in `find $OUTPUTFOLDER/templates -iname "*.xml"` ; do
        echo -e $graystart "translate $f"  $colorend
       translate $f ;
    done
}

# to lower case
for searchFolder in config templates actions images workspace; do
    lowerCaseRecurse $OUTPUTFOLDER/$searchFolder;
done

# copy binaries
BINARIES="$BUILDPATH/app/HMI $BUILDPATH/app/HMILogger $BUILDPATH/app/HMIWdDialog $BUILDPATH/app/libHMIWebBrowserPlugin.so $BUILDPATH/app/libHMITextEditorPlugin.so $BUILDPATH/app/HMIUpdater"

if [ -n "$JM_PLATFORM" -a -d "../qthmi/Support/libraries/${JM_PLATFORM}/release" ]; then
	cp $BINARIES $OUTPUTFOLDER
	cp $BUILDPATH/app/*.so.[0-9] $OUTPUTFOLDER
	cp $BUILDPATH/app/*.so $OUTPUTFOLDER # libuastack.so
	cp -r ../qthmi/Support/libraries/${JM_PLATFORM}/release/* $OUTPUTFOLDER
else
    for bin in $BINARIES ; do
        copyBin $bin
    done

    # copy Qt plugins
    copyQtPlugins
fi

# copy libqftp if any
echo "Copying files $BUILDPATH/app/libQtFtp and libQtHttp... "
cp $BUILDPATH/app/libQtFtp.so $OUTPUTFOLDER
cp $BUILDPATH/app/libQtFtp.so.[0-9] $OUTPUTFOLDER
cp $BUILDPATH/app/libQtHttp.so $OUTPUTFOLDER
cp $BUILDPATH/app/libQtHttp.so.[0-9] $OUTPUTFOLDER

# copy about file
echo "Copying about file $BUILDPATH/app/$HMIABOUTDIR/libHMIAbout.so.1 ... "
cp -L $BUILDPATH/app/$HMIABOUTDIR/libHMIAbout.so.1 $OUTPUTFOLDER || exit 1
HMIVERSION=$(strings $OUTPUTFOLDER/libHMIAbout.so.1 | grep Build | sed 's/) - Build (/\./' | sed 's/)//' | sed 's/[^0.9](/\./')

mkdir $OUTPUTFOLDER/protocols

# Do not copy protocols. The Studio will download them according to the target
# with the first project
#cp -a $BUILDPATH/app/protocols/lib*.so $OUTPUTFOLDER/protocols

mkdir $OUTPUTFOLDER/modules
cp -a $BUILDPATH/app/modules/lib*.so $OUTPUTFOLDER/modules

if [[ $PLATFORM == "Linux OE/usom01" ]] ; then
    BRANDNAME=exor
    translate_files
    echo -e $cyanstart "Replacing default web port to 8000" $1 $colorend
    sed -i 's/<port>80</<port>8000</' $OUTPUTFOLDER/config/httpd_config.xml
    sed -i 's,<listeningAddress.*$,<listeningAddress>127.0.0.1</listeningAddress>,' $OUTPUTFOLDER/config/httpd_config.xml
fi

if [[ $PLATFORM == "Linux OE/simrad" ]] ; then
    BRANDNAME=naviop
    translate_files

    mv $OUTPUTFOLDER/config/httpd_config_portable.xml $OUTPUTFOLDER/config/httpd_config.xml
    mv $OUTPUTFOLDER/config/ftpdconfig_portable.xml $OUTPUTFOLDER/config/ftpdconfig.xml
    mv $OUTPUTFOLDER/config/httpd_security_config_portable.xml $OUTPUTFOLDER/config/httpd_security_config.xml
    mv $OUTPUTFOLDER/config/system_portable.xml $OUTPUTFOLDER/config/system.xml
    cp ../qthmi/Support/install/files/naviop/config/jmshadow $OUTPUTFOLDER/config/jmshadow

    # Install CDS3
    mkdir $OUTPUTFOLDER/rts
    cp -r $CDS3_DIR/RTS/UN60/* $OUTPUTFOLDER/rts
    cp -r $CDS3_DIR/RTS/Brands/NaviOP/Linux/* $OUTPUTFOLDER/rts
fi

if [[ $PLATFORM == "Linux OE/hetronic" ]] ; then
    BRANDNAME=hetronic
    translate_files

    mv $OUTPUTFOLDER/config/httpd_config_portable.xml $OUTPUTFOLDER/config/httpd_config.xml
    mv $OUTPUTFOLDER/config/ftpdconfig_portable.xml $OUTPUTFOLDER/config/ftpdconfig.xml
    mv $OUTPUTFOLDER/config/httpd_security_config_portable.xml $OUTPUTFOLDER/config/httpd_security_config.xml
    mv $OUTPUTFOLDER/config/system_portable.xml $OUTPUTFOLDER/config/system.xml
fi

if [[ $PLATFORM == "Linux OE/devkit" ]] ; then
    BRANDNAME=openhmi
    translate_files

    mv $OUTPUTFOLDER/config/httpd_config_portable.xml $OUTPUTFOLDER/config/httpd_config.xml
    mv $OUTPUTFOLDER/config/ftpdconfig_portable.xml $OUTPUTFOLDER/config/ftpdconfig.xml
    mv $OUTPUTFOLDER/config/httpd_security_config_portable.xml $OUTPUTFOLDER/config/httpd_security_config.xml
    mv $OUTPUTFOLDER/config/system_portable.xml $OUTPUTFOLDER/config/system.xml

    # Install CDS3
    mkdir $OUTPUTFOLDER/rts
    cp -r $CDS3_DIR/RTS/UN60/* $OUTPUTFOLDER/rts
    cp -r $CDS3_DIR/RTS/Brands/OpenHMI/Linux/* $OUTPUTFOLDER/rts
fi

if [[ $PLATFORM == "Linux OE/usom01-portable" ]] ; then
    BRANDNAME=openhmi
    translate_files

    mv $OUTPUTFOLDER/config/httpd_config_portable.xml $OUTPUTFOLDER/config/httpd_config.xml
    mv $OUTPUTFOLDER/config/ftpdconfig_portable.xml $OUTPUTFOLDER/config/ftpdconfig.xml
    mv $OUTPUTFOLDER/config/httpd_security_config_portable.xml $OUTPUTFOLDER/config/httpd_security_config.xml
    mv $OUTPUTFOLDER/config/system_portable.xml $OUTPUTFOLDER/config/system.xml

    # Install CDS3
    mkdir $OUTPUTFOLDER/rts
    cp -r $CDS3_DIR/RTS/UN60/* $OUTPUTFOLDER/rts
    cp -r $CDS3_DIR/RTS/Brands/OpenHMI/Linux/* $OUTPUTFOLDER/rts
fi

if [[ $PLATFORM == "Linux/x86-32" ]] ; then
    BRANDNAME=exor
    translate_files

    mv $OUTPUTFOLDER/config/httpd_config_portable.xml $OUTPUTFOLDER/config/httpd_config.xml
    mv $OUTPUTFOLDER/config/ftpdconfig_portable.xml $OUTPUTFOLDER/config/ftpdconfig.xml
    mv $OUTPUTFOLDER/config/httpd_security_config_portable.xml $OUTPUTFOLDER/config/httpd_security_config.xml
    mv $OUTPUTFOLDER/config/system_portable.xml $OUTPUTFOLDER/config/system.xml

    ./jenkins_jmservices_build.sh || die 1;

    # prepare EPAD
    rm -rf epad-system-root
    mkdir -p epad-system-root/usr/bin
    mkdir -p epad-system-root/usr/lib
    mkdir -p epad-system-root/usr/share/dbus-1/system-services
    mkdir -p epad-system-root/usr/share/dbus-1/services
    mkdir -p epad-system-root/etc/dbus-1/system.d
    cp -a ltools-epad/build-generic-release/{EPAD,showmessage,filebrowser,vkeyboard} epad-system-root/usr/bin || die 1;
    cp -a ltools-epad/chpass.sh                                epad-system-root/usr/bin || die 1;
    cp -a ltools-epad/com.exor.EPAD.service                    epad-system-root/usr/share/dbus-1/system-services || die 1;
    cp -a ltools-epad/filebrowser/com.exor.FileBrowser.service epad-system-root/usr/share/dbus-1/services || die 1;
    cp -a ltools-epad/vkeyboard/com.exor.VKeyboard.service     epad-system-root/usr/share/dbus-1/services || die 1;
    cp -a ltools-epad/showmessage/com.exor.ShowMessage.service epad-system-root/usr/share/dbus-1/services || die 1;
    cp -a ltools-epad/com.exor.EPAD.conf                       epad-system-root/etc/dbus-1/system.d || die 1;
    cd epad-system-root
    tar cvzf ../epad-$1.tgz . || die 1
    cd ..


    # prepare jmlauncher
    rm -rf jml-system-root
    mkdir -p jml-system-root/usr/bin
    mkdir -p jml-system-root/usr/lib
    mkdir -p jml-system-root/usr/share/dbus-1/system-services
    mkdir -p jml-system-root/usr/share/dbus-1/services
    mkdir -p jml-system-root/etc/dbus-1/system.d
    cp -a jml/build-generic-release/jmlauncher_daemon/jmlauncher jml-system-root/usr/bin/ || die 1
    cp -a jml/build-generic-release/jmlauncher_ui/jmlauncherUI   jml-system-root/usr/bin/ || die 1
    cp -a jml/jmlauncher_daemon/com.exor.JMLauncher.service      jml-system-root/usr/share/dbus-1/system-services/ || die 1
    cp -a jml/jmlauncher_daemon/com.exor.JMLauncher.conf         jml-system-root/etc/dbus-1/system.d/ || die 1
    cd jml-system-root
    tar cvzf ../jml-$1.tgz . || die 1
    cd ..


fi


# create hmiversion.xml
echo -e $yellowstart "Creating version file" $colorend
chmod 0777 createversion.sh
./createversion.sh $OUTPUTFOLDER

# create pgk
createPkg ./scripts $2;

if [[ $PLATFORM == "Linux/x86-32" ]] ; then
    BRANDNAME=exor

    ./jenkins_jmdocker_build.sh
fi
