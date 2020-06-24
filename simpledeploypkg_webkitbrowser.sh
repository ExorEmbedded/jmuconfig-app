#!/bin/bash

# Scans xml files and create deploy folder
# search path format is <search folter>:<search depth>
#
# NOTE: requires xmlpatterns tool from qt


cd `dirname $0`

HMIVERSION="`cat version.h | grep -o '[0-9.]\+'`"

cd linuxbuild

PKG_PREFIX="webkit-$HMIVERSION"

# load common functions
source ./simpledeploypkg_qt.sh


# copy start script
cp ../scripts/start.sh $OUTPUTFOLDER
chmod 0777 $OUTPUTFOLDER/start.sh


# copy binaries
BINARIES="$BUILDPATH/WebkitBrowser"
for bin in $BINARIES ; do
	copyBin $bin 
done

# copy Qt plugins
copyQtPlugins;


#echo "Applying version $VERSION"
#sed -i "s/<version>.*<\\\/version>/<version>$VERSION<\\\/version>/" ../scripts/package.info
cp ../scripts/package.info $OUTPUTFOLDER
cp ../scripts/browser.ini $OUTPUTFOLDER

# create pgk
createPkg ../scripts $2;


