#!/bin/bash

# To invoke outside jenkins:
# ONLY_DELIVERING=false BUILD_FOR_LINUX=true PLATFORMS=UN60,UN69,LIN32 ./jenkins_linux_build.sh

pushd .

cd ..

# Expects to have the delivering share in parent folder
baseDir="$(pwd)"
deliveringDir="$(pwd)/../delivering";
portableCreatorDir="$(pwd)/../../jmportablecreator";

function quit {
   if [ ! "$PLATFORMS" = "" ]; then
      [ "$1" = "0"  ] && echo "set linux=OK" > linuxBuildResult.txt || echo "set linux=FAIL" > linuxBuildResult.txt;
   fi
   # workaround for locking issues in samba mount
   mv linuxBuildResult.txt $deliveringDir/linuxBuild.bat
   exit $1

   if [ ! "$BUILD_FOR_LINUX" = "" ]; then
      [ "$1" = "0"  ] && echo "set linux=OK" > linuxBuildResult.txt || echo "set linux=FAIL" > linuxBuildResult.txt;
   fi
   # workaround for locking issues in samba mount
   mv linuxBuildResult.txt $deliveringDir/linuxBuild.bat
   exit $1
}

[ "$BUILD_FOR_LINUX" = "false" ] && quit 0

major="$( cat jbuild/version.proj | awk '/Major / {print}' | sed 's/\(^.*">\|<\/.*$\)//g' )";
minor="$( cat jbuild/version.proj | awk '/Minor / {print}' | sed 's/\(^.*">\|<\/.*$\)//g' )";
sp="$( cat jbuild/version.proj | awk '/SP / {print}' | sed 's/\(^.*">\|<\/.*$\)//g' )";
b="$( cat jbuild/version.proj | awk '/Build / {print}' | sed 's/\(^.*">\|<\/.*$\)//g' )";
fver="$( cat qthmi/QTHmi/Include/veridrc.h | awk '/_szVerId_FILEVERSION/ {print}' | sed 's/\(^.* "\|"\)//g' )";

echo "version: $major.$minor.$sp.$b";

popd
