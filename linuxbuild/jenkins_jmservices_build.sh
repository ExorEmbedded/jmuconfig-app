#!/bin/bash

# run via docker:

# To build via docker:
# (from jenkins-pipelines/dockerfile repo )
# docker build -f bionic-QT5-x86-64-ssl1.0  . -t jmobile-build-env
# ocker run --rm -ti -v $(pwd)/../:/build  -v /opt:/opt  -v /home:/home -v /etc/passwd:/etc/passwd -u $(id -u):$(id -g) jmobile-build-env /bin/bash -c "/build/linuxbuild/jenkins_jmservices_build.sh"

cd $(dirname $0)

. ./common.sh

EPAD_REV=5b974dec3d7fb3a63f0ead482b96f36f92f66fb5
JML_REV=667d92440335e71e0a985f034cf42f1bec227dc4
QtPath=/opt/exor/Qt484-rel-mt
QMakePath=$QtPath/bin/qmake

git clone git@bitbucket.exorint.cloud:bsp/ltools-epad

pushd .
    cd ltools-epad
    git fetch --all
    git checkout $EPAD_REV || quit 1;
    $QMakePath -r epad.pro || quit 1;
    cd impl/generic/
    make || quit 1;
    cd ../../
    make -j -f Makefile.app || quit 1;
    cd filebrowser && make -f Makefile.filebrowser || quit 1;
    cd ..
    cd showmessage && make -f Makefile.showmessage || quit 1;
    cd ..
    cd vkeyboard && make -f Makefile.vkeyboard || quit 1;
    cd ..
    #cd gtk-im
    #$QMakePath -r || quit 1;
    #make -j || quit 1;
    #cd ..

popd

git clone git@bitbucket.exorint.cloud:bsp/jml

pushd .
    cd jml
    git fetch --all
    git checkout $JML_REV || quit 1
    mkdir -p build-generic-release
    cd build-generic-release
    $QMakePath -r ../jmlauncher.pro
    make -j
popd

