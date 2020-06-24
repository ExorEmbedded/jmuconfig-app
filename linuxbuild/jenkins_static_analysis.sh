#!/bin/bash

# To invoke outside jenkins:
# ONLY_DELIVERING=false BUILD_FOR_LINUX=true PLATFORMS=UN60,UN69,LIN32 ./jenkins_linux_build.sh

# To build via docker:
# (from jenkins-pipelines/dockerfile repo )
# docker build -f bionic-QT5-x86-64-ssl1.0  . -t jmobile-build-env
# docker run --rm -ti  -v /opt:/opt  -v /home:/home -v /etc/passwd:/etc/passwd -u $(id -u):$(id -g) jmobile-build-env /bin/bash -c /home/autosvn/workspace/jmobile_linux_build/src/linuxbuild/jenkins_static_analysis.sh

cd $(dirname $0)
cd ..

# Expects to have the delivering share in parent folder
baseDir="$(pwd)"
deliveringDir="$(pwd)/../delivering";
portableCreatorDir="$(pwd)/../../jmportablecreator";


SONAR_SERVER=http://192.168.20.178:9000/
SONAR_TOKEN=3557844ea215228038928af41aa320d92ceabf2f
SONAR_PROJECT=JMobileRuntime
PLATFORMS="LIN32"

function quit {
   echo "GOING TO EXIT...."
   read
   exit $1
}


# Check if anything to build on linux
BUILD_FOR_LINUX="false"


IFS="$IFS," ; for PLATFORM in $PLATFORMS ; do

	case $PLATFORM in
		"UN60_QT5")
			PlatformDir="un60_qt5_linuxoe";
			BuildDir=$(pwd)/build5-linx11-usom01-release;
                        QtPath=/opt/exor/Qt512-arm-exor
                        QtNativePath=/opt/exor/Qt512-x86-64-exor
			QMakePath=$QtPath/bin/qmake
			StripPath=arm-poky-linux-gnueabi-strip
                        QMakeParms='CONFIG+=release CONFIG+=usom01 OS=linx11 CONFIG+=staticidal'
			SDKNATIVESYSROOT=/opt/exorintos/2.6.1/sysroots/i686-pokysdk-linux
			export PATH=$SDKNATIVESYSROOT/bin:$SDKNATIVESYSROOT/usr/bin:$SDKNATIVESYSROOT/usr/bin/arm-poky-linux-gnueabi:$PATH:$QtNativePath/bin
			BUILD_FOR_LINUX="true"
			;;
		"UN60")
			PlatformDir="un60_linuxoe";
			BuildDir=$(pwd)/build-linx11-usom01-release"";
                        QtPath=/opt/exor/Qt484-usom01-O3-rel-mt
                        QtNativePath=/opt/exor/Qt512-x86-64-exor
			QMakePath=$QtPath/bin/qmake
			StripPath=arm-poky-linux-gnueabi-strip
                        QMakeParms='CONFIG+=release CONFIG+=usom01 OS=linx11 CONFIG+=staticidal'
			SDKNATIVESYSROOT=/opt/exorintos/1.5.3/sysroots/i686-pokysdk-linux
			export PATH=$SDKNATIVESYSROOT/bin:$SDKNATIVESYSROOT/usr/bin:$SDKNATIVESYSROOT/usr/bin/arm-poky-linux-gnueabi:$QtPath/bin:$PATH:$QtNativePath/bin
			BUILD_FOR_LINUX="true"
			;;
                "UN69")
                        PlatformDir="un69_linuxoe";
                        BuildDir=$(pwd)/build-linx11-usom01-release"";
                        RuntimeDir=QtHmi/runtime/$PlatformDir;
			BUILD_FOR_LINUX="true"
                        pushd .
                        cd ../delivering
                        echo "Delivering $PlatformDir ...";
                        mkdir -p $RuntimeDir
                        cp -fr $BuildDir/../linuxbuild/scripts_un69/*.sh $RuntimeDir/ || quit 1;
                        mkdir -p $RuntimeDir/deploy/
                        mv -f $RuntimeDir/start.sh $RuntimeDir/deploy/ || quit 1;
                        popd
                        continue;
                        ;;
                "LIN32")
			PlatformDir="lin32";
			BuildDir=$(pwd)/build-linx11-x86-debug"";
                        QtPath=/opt/exor/Qt484-rel-mt
			QMakePath=$QtPath/bin/qmake
			StripPath=strip
                        QMakeParms='CONFIG-=release CONFIG+=debug CONFIG+=x86 OS=linx11 CONFIG+=staticidal'
			BUILD_FOR_LINUX="true"
			;;
		*)
			echo "Unknown platform $PLATFORM!";
			continue;
			;;
	esac

	# creates build dir
       [ ! -d $BuildDir/build ] && mkdir -p $BuildDir/build

       echo "Building $PLATFORM platform...";
       pushd .
       cd $BuildDir/build/;
       mkdir -p  ../../../delivering/clang || quit 1

       if [ "$ONLY_DELIVERING" = "false" ] ;  then

           echo $QMakeParms | xargs $QMakePath -r ../../linuxbuild/viewertest.pro;
           [ ! "$?" = "0" ] && quit 1
           # need to force clang compiler for bulding since g++ fails when compiling precompiled headers (the scanner generates gcc like pch, but when compiling clang style pch are expected )
           which clang-8 || quit 1
           which clang++-8 || quit 1
           SRC_DIR="`readlink -f $(pwd)/../../`"

           # clang-tidy build
           # clang-tidy step1: create a valid compilation database from clang
           mkdir -p /tmp/compile_commands
           mkdir -p $(pwd)/../../../delivering/clang-tidy/
           rm -rf $(pwd)/../../../delivering/clang-tidy/*
           rm -rf /tmp/compile_commands/*
           make clean
           make -i -j1 CXX='clang++-8 -m32 -MJ /tmp/compile_commands/$(shell date +%s%N).json '  CC='clang-8 -m32 -MJ /tmp/compile_commands/$(shell date +%s%N).json '
           # clang-tidy step2: merge fragments into final compilation database
           sed -e '1s/^/[\n/' -e '$s/,$/\n]/' /tmp/compile_commands/*.json > $(pwd)/../../../delivering/clang-tidy/compile_commands.json

           # run clang-tidy from the created compilation database
           run-clang-tidy-8 -checks="cert-*" -p $(pwd)/../../../delivering/clang-tidy > $(pwd)/../../../delivering/clang-tidy/compile.log 2>&1
           sed -i "s,^[ ]*\.\./[^ ]*\(/\(qthmi\|idal\)/[^:]*\),$SRC_DIR\1," ../../../delivering/clang-tidy/compile.log


           # fix relative paths in compile_commands.json, or sonar-cxx cannot find it
           rm -rf ../../../delivering/clang/*
           $baseDir/linuxbuild/fix_rel_paths.py ../../../delivering/clang-tidy/compile_commands.json  ../../../delivering/clang/compile_commands.json
           make clean
           scan-build-8 -plist-html --use-cc=clang-8 --use-c++=clang++-8   -o ../../../delivering/clang/ make  -i -j2;

           # fix relative paths to absolute in plist files (otherwise sonar cannot find a match )
           sed -i "s,<string>\..*\(/\(qthmi\|idal\)/.*\)</string>,<string>$SRC_DIR\1</string>," `find ../../../delivering/clang/ -iname "*.plist"`

       fi #only delivering

       /sonar-scanner/bin/sonar-scanner -Dsonar.projectKey=$SONAR_PROJECT -Dsonar.host.url=$SONAR_SERVER -Dsonar.login=$SONAR_TOKEN \
            -Dsonar.projectBaseDir=$(pwd)/../../  -Dsonar.sources=./idal/,qthmi/ \
            -Dsonar.exclusions=qthmi/QTHmi/3rdParty/**,idal/protocols2/3rd-party/**,qthmi/Support/**,qthmi/Qt_Shadowbuild/**,idal/icu/gprs/test/**,**/*.java,**/*.jar,**/*.vcproj,**/*.svg,**/*.js,**/*.ts,**/*.html,**/*.xml \
            -Dsonar.cxx.jsonCompilationDatabase=$(pwd)/../../../delivering/clang/compile_commands.json -Dsonar.cxx.scanOnlySpecifiedSources=True  \
            -Dsonar.cxx.clangsa.reportPath=$(pwd)/../../../delivering/clang/*/*.plist -Dsonar.cxx.clangtidy.reportPath=$(pwd)/../../../delivering/clang-tidy/compile.log

       [ ! "$?" = "0" ] && quit 1
       popd;

done

# just quit if no platform built
[ "$BUILD_FOR_LINUX" = "false" ] && quit 0


echo All done!;
quit 0
