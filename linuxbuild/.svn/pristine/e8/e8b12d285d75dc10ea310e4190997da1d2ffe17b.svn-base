#!/bin/bash

cd `dirname $0`
DIR="`pwd`"
echo "Working directory: $DIR"
DIRLOWER="`echo $DIR | tr '[:upper:]' '[:lower:]' `"


cd embedxml
PATH=/opt/exor/Qt484/bin:$PATH LD_LIBRARY_PATH=/opt/exor/Qt484/lib:$LD_LIBRARY_PATH qmake
PATH=/opt/exor/Qt484/bin:$PATH LD_LIBRARY_PATH=/opt/exor/Qt484/lib:$LD_LIBRARY_PATH make
sudo cp EmbedXml /usr/bin/
cd ../


SHADOWBUILDPATH=

case $1 in
    altera)
        export PATH=/opt/exor/Qt484e-altera/bin:$PATH
        export LD_LIBRARY_PATH=/opt/exor/Qt484e-altera/lib:$LD_LIBRARY_PATH
        SHADOWBUILDPATH=../build-linqws-un31/
        ;;
    *)
        export PATH=/opt/exor/Qt484/bin:$PATH
        export LD_LIBRARY_PATH=/opt/exor/Qt484/lib:$LD_LIBRARY_PATH
        SHADOWBUILDPATH=../build-linx11-x86/
        ;;
esac

if [ "$DIRLOWER" == "$DIR" ]; then
   echo "OK"
else
  echo "Current working directory $DIR must contain only lowercase characters."
  exit
fi

echo "Build using qmake `which qmake`"
mkdir -p $SHADOWBUILDPATH-debug/build
cd  $SHADOWBUILDPATH-debug/build
/opt/exor/Qt4.8.4/bin/qmake -r CONFIG+=debug ../../../linuxbuild/viewertest.pro
make -j2

