#!/bin/bash

# To invoke outside jenkins:
# ONLY_DELIVERING=false BUILD_FOR_LINUX=true PLATFORMS=UN60,UN69,LIN32 ./jenkins_linux_build.sh

# To build via docker:
# (from jenkins-pipelines/dockerfile repo )
# docker build -f bionic-QT5-x86-64-ssl1.0  . -t jmobile-build-env

. ./common.sh

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


#if [ "$ONLY_DELIVERING" = "false" ]; then
#   if [ "$SKIP_SVN_UPDATE" = "false" ]; then
#
#      [ "$REBUILD_ALL" =  "true" ] && svn-clean
#      echo "Updating svn...";
#      svn cleanup;
#      svn revert -R .;
#      if [ "$REBUILD_ALL" = "true" ]; then
#         svn-clean;
#         mkdir -p build-linx11-usom01-release;
#         mkdir -p build-linx11-x86-release;
#      fi
#      svn update   --username "autosvn" --password "NpkKFuwH5kfYxn" ;
#   fi
#fi

# Check if anything to build on linux
BUILD_FOR_LINUX="false"


IFS="$IFS," ; for PLATFORM in $PLATFORMS ; do

	case $PLATFORM in
		"UN60_3.0")
			PlatformDir="un60_3.0_linuxoe";
			BuildDir=$(pwd)/build5-3.0-usom01-release;
                        QtPath=/opt/exor/Qt512-arm-exor-3.0
                        QtNativePath=/opt/exor/Qt512-x86-exor
			QMakePath=$QtPath/bin/qmake
			StripPath=arm-poky-linux-gnueabi-strip
                        QMakeParms='CONFIG+=release CONFIG+=usom01 OS=3.0 CONFIG+=staticidal'
			SDKNATIVESYSROOT=/opt/exorintos-1.3.x/1.3.x/sysroots/x86_64-pokysdk-linux
			export PATH=$SDKNATIVESYSROOT/bin:$SDKNATIVESYSROOT/usr/bin:$SDKNATIVESYSROOT/usr/bin/arm-poky-linux-gnueabi:$PATH:$QtNativePath/bin
			BUILD_FOR_LINUX="true"
			;;
		"UN78_3.0")
			PlatformDir="un78_3.0_linuxoe";
			BuildDir=$(pwd)/build5-3.0-usom04-release;
                        QtPath=/opt/exor/Qt512-arm64-exor-3.0
                        QtNativePath=/opt/exor/Qt512-x86-exor
			QMakePath=$QtPath/bin/qmake
			StripPath=aarch64-poky-linux-strip
                        QMakeParms='CONFIG+=release CONFIG+=usom04 OS=3.0 CONFIG+=staticidal'
			SDKNATIVESYSROOT=/opt/exorintos-2.0.x/2.0.x/sysroots/x86_64-pokysdk-linux
			export PATH=$SDKNATIVESYSROOT/bin:$SDKNATIVESYSROOT/usr/bin:$SDKNATIVESYSROOT/usr/bin/aarch64-poky-linux:$PATH:$QtNativePath/bin
			BUILD_FOR_LINUX="true"
			;;
		"UN60_QT5")
			PlatformDir="un60_qt5_linuxoe";
			BuildDir=$(pwd)/build5-linx11-usom01-release;
                        QtPath=/opt/exor/Qt512-arm-exor
                        QtNativePath=/opt/exor/Qt512-x86-exor
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
                        QtNativePath=/opt/exor/Qt512-x86-exor
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
			BuildDir=$(pwd)/build-linx11-x86-release"";
                        QtPath=/opt/exor/Qt484-rel-mt
			QMakePath=$QtPath/bin/qmake
			StripPath=strip
                        QMakeParms='CONFIG+=release CONFIG+=x86 OS=linx11 CONFIG+=staticidal'
			BUILD_FOR_LINUX="true"
			;;
			    "LIN64")
			PlatformDir="lin64";
			BuildDir=$(pwd)/build5-linwayland-x64-release"";
                        QtPath=/opt/exor/Qt512-x86-64-exor
			QMakePath=$QtPath/bin/qmake
			StripPath=strip
                        QMakeParms='CONFIG+=release CONFIG+=x64 OS=linwayland CONFIG+=staticidal'
			BUILD_FOR_LINUX="true"
			;;
		*)
			echo "Unknown platform $PLATFORM!";
			continue;
			;;
	esac

	# creates build dir
       [ ! -d $BuildDir/build ] && mkdir -p $BuildDir/build

       if [ "$ONLY_DELIVERING" = "false" ]; then
           echo "Building $PLATFORM platform...";
           pushd .
           cd $BuildDir/build/;
           echo $QMakeParms | xargs $QMakePath -r ../../linuxbuild/viewertest.pro;
           [ ! "$?" = "0" ] && quit 1
           make -j2;
           [ ! "$?" = "0" ] && quit 1
           popd;
        fi

	# save current main folder and switch to delivering folder
	pushd .
	cd ../delivering

	echo "Delivering $PlatformDir ...";
	RuntimeDir=QtHmi/runtime/$PlatformDir;

	# Copy protocols
	mkdir -p QtHmi/target/protocols/$PlatformDir;
	cp -fr $BuildDir/build/app/protocols/* QtHmi/target/protocols/$PlatformDir;

	# Copy folders KeyPadTemplates and UserMgmtTemplates
	mkdir -p $RuntimeDir/templates/keypadtemplates;
	for f in $( cd $BuildDir/../qthmi/Support/templates/KeyPadTemplates/; find . ! -path . ); do
	   fl=$( echo $f | tr '[:upper:]' '[:lower:]' );
	   [ -d $BuildDir/../qthmi/Support/templates/KeyPadTemplates/$f ] && mkdir -p $RuntimeDir/templates/keypadtemplates/$fl && continue;
	   cp -f $BuildDir/../qthmi/Support/templates/KeyPadTemplates/$f $RuntimeDir/templates/keypadtemplates/$fl;
	done;
	mkdir -p $RuntimeDir/templates/usermgmttemplates;
	for f in $( cd $BuildDir/../qthmi/Support/install/files/exor/UserMgmtTemplates/; find . ! -path . ); do
	   fl=$( echo $f | tr '[:upper:]' '[:lower:]' );
	   [ -d $BuildDir/../qthmi/Support/install/files/exor/UserMgmtTemplates/$f ] && mkdir -p $RuntimeDir/templates/usermgmttemplates/$fl && continue;
	   cp -f $BuildDir/../qthmi/Support/install/files/exor/UserMgmtTemplates/$f $RuntimeDir/templates/usermgmttemplates/$fl;
	done;

	# Copy build files
	#for f in $( ls $BuildDir/build/app | cat | sed '/^libHMIAbout/d' | sed '/.so$/d' | sed '/^protocols$/d' | sed '/so.[0-9]./d'); do
	#   cp -frL $BuildDir/build/app/$f $RuntimeDir/;
	#done;

	mkdir -p $RuntimeDir/modules;

	$StripPath $BuildDir/build/app/HMI
	cp -fL $BuildDir/build/app/HMI $RuntimeDir/ || quit 1;
	$StripPath $BuildDir/build/app/HMILogger
	cp -fL $BuildDir/build/app/HMILogger $RuntimeDir/ || quit 1;
	$StripPath $BuildDir/build/app/HMIWdDialog
	cp -fL $BuildDir/build/app/HMIWdDialog $RuntimeDir/ || quit 1;
	$StripPath $BuildDir/build/app/HMIUpdater
	cp -fL $BuildDir/build/app/HMIUpdater $RuntimeDir/ || quit 1;
	$StripPath $BuildDir/build/app/BackUp
	cp -fL $BuildDir/build/app/BackUp $RuntimeDir/ || quit 1;
	cp -fL $BuildDir/build/app/libHMIIdalInterface.so.1 $RuntimeDir/ #|| quit 1;
	cp -fL $BuildDir/build/app/libomnithread.so.1 $RuntimeDir/ || quit 1;
	$StripPath $BuildDir/build/app/libHMITextEditorPlugin.so
	cp -fL $BuildDir/build/app/libHMITextEditorPlugin.so $RuntimeDir/ || quit 1;
	$StripPath $BuildDir/build/app/libqjson.so
	cp -fL $BuildDir/build/app/libqjson.so.1 $RuntimeDir/ || quit 1;
	$StripPath $BuildDir/build/app/modules/libopcua.so
	cp -fL $BuildDir/build/app/modules/libopcua.so $RuntimeDir/modules/ #|| quit 1;
	$StripPath $BuildDir/build/app/modules/libcorvina.so
	cp -fL $BuildDir/build/app/modules/libcorvina.so $RuntimeDir/modules/ #|| quit 1;
	$StripPath $BuildDir/build/app/modules/libifmqtt*.so
	cp -fL $BuildDir/build/app/modules/libifmqtt*.so $RuntimeDir/modules/ #|| quit 1;
	$StripPath $BuildDir/build/app/libHMIWebBrowserPlugin.so
	cp -fL $BuildDir/build/app/libHMIWebBrowserPlugin.so $RuntimeDir/ #|| quit 1;

	if [ "$PLATFORM" != "UN60_QT5" -a "$PLATFORM" != "UN60_3.0" -a "$PLATFORM" != "UN78_3.0" ]; then
		$StripPath $BuildDir/build/app/HTTPUpdateUtility
		cp -fL $BuildDir/build/app/HTTPUpdateUtility $RuntimeDir/ || quit 1;		
	else
		$StripPath $BuildDir/build/app/libQtFtp.so.1
		cp -fL $BuildDir/build/app/libQtFtp.so.1 $RuntimeDir/ || quit 1
	fi
	cp -fL $BuildDir/build/app/HMIClient $RuntimeDir/ #|| quit 1;
	
	mkdir -p $RuntimeDir/deploy;

	# Copy scripts
	cp -fr $BuildDir/../linuxbuild/scripts/*.sh $RuntimeDir/ || quit 1;
	mv -f $RuntimeDir/start.sh $RuntimeDir/deploy/ || quit 1;

	rm -f $RuntimeDir/package.info;
	(
	   IFS=$'\n' ; # temporarily use line separator as field separator
	   for line in $( cat $BuildDir/../linuxbuild/scripts/package.info ); do
		v=$( echo $line | sed s/[0-9]\.[0-9]\.[0-9]\.[0-9]/$fver/ );
		echo $v >> $RuntimeDir/package.info;
	   done;
	);

	rm -f $RuntimeDir/clientpackage.info;
	(
	   IFS=$'\n' ; # temporarily use line separator as field separator
	   for line in $( cat $BuildDir/../linuxbuild/scripts/clientpackage.info ); do
		v=$( echo $line | sed s/[0-9]\.[0-9]\.[0-9]\.[0-9]/$fver/ );
		echo $v >> $RuntimeDir/clientpackage.info;
	   done;
	);

	# Copy run.sh for client
	mkdir -p $RuntimeDir/clientscripts;
	cp -fr $BuildDir/../linuxbuild/scripts_client/clientrun.sh $RuntimeDir/clientscripts || quit 1;

	# Copy config, actions and workspace dir
	for f in $( cd $BuildDir/../qthmi/Support/misc/target; find . ! -path . ); do
	   fl=$( echo $f | tr '[:upper:]' '[:lower:]' );
	   [ -d $BuildDir/../qthmi/Support/misc/target/$f ] && mkdir -p $RuntimeDir/$fl && continue;
	   cp -f $BuildDir/../qthmi/Support/misc/target/$f $RuntimeDir/$fl || quit 1;
	done;

	# Copy Image dir
	for f in $( cd $BuildDir/../qthmi/Support/Images/; find . ! -path . ); do
	   fl=$( echo $f | tr '[:upper:]' '[:lower:]' );
	   mkdir -p $RuntimeDir/deploy/images;
	   cp -f $BuildDir/../qthmi/Support/Images/$f $RuntimeDir/deploy/images/$fl || quit 1;
	done;

	# Copy libraries
	cp -fr `find $BuildDir/../qthmi/Support/libraries/ -iname $PlatformDir`/release/* $RuntimeDir/deploy ||  quit 1;

	# Branding
	if [ ! -z "$BRANDS" ]; then
	   pushd .
	   cd $BuildDir/build;
	   for brand in $( echo $BRANDS | sed 's/,/ /g' ); do
                  mkdir -p brands/$brand
		  cd brands/$brand
		  echo $QMakeParms | xargs $QMakePath  ../../../../qthmi/QTHmi/HMIAbout/HMIAbout.pro CONFIG+=$brand;
		  [ ! "$?" = "0" ] && quit 1
		  make -j2;
		  [ ! "$?" = "0" ] && quit 1

		  mkdir -p ../../../../../delivering/QtHmi_$brand/runtime/$PlatformDir/deploy;
		  cp -fL ../../app/$brand/libHMIAbout.so.1 ../../../../../delivering/QtHmi_$brand/runtime/$PlatformDir/deploy/ || quit 1;

		# build docker
		if [ $brand = "exor" -a "$PLATFORM" == "LIN32" ]; then
			pushd .
			cd $baseDir/linuxbuild
			./simpledeploypkg_jmobile.sh pc || quit 1
			gzip jmdocker.tar || quit 1
			mv jmdocker.tar.gz $baseDir/../delivering/Exor/jmdocker-v$major.$minor.$sp.$b.tar.gz || quit 1
			popd
		fi

		if [ $brand = "naviop" -a "$CREATE_NAVIOP_PORTABLE" = "true" ]; then
			cd $baseDir/linuxbuild

			if [ "$PLATFORM" == "UN60_QT5" ]; then
				./simpledeploypkg_jmobile.sh simrad-qt5 || quit 1
				rm -rf deploy .tmppackage
				mv jm-simrad-qt5.tgz $portableCreatorDir

				cd $portableCreatorDir
				git checkout .
				git checkout wayland
				git pull

                	        rm -rf mainos
				fakeroot -- /bin/bash -c "\
					mkdir mainos; \
                       	 	tar xzf mainos-wayland.tar.gz -C mainos; \
					echo k | ./portable_deploy_simrad.sh; \
				"
				[ ! "$?" = "0" ] && quit 1
				cp jmobile_portable_simrad.tar.gz $baseDir/../delivering/NaviOP/N-Design_Runtime_QT5-v$major.$minor.$sp.$b.tar.gz || quit 1
			else
				./simpledeploypkg_jmobile.sh simrad || quit 1
				rm -rf deploy .tmppackage
				mv jm-simrad.tgz $portableCreatorDir

				cd $portableCreatorDir
				git checkout .
				git checkout master
				git pull

                        	rm -rf mainos
				fakeroot -- /bin/bash -c "\
					mkdir mainos; \
                        		tar xzf mainos.tar.gz -C mainos; \
					echo k | ./portable_deploy_simrad.sh; \
                        	"
				[ ! "$?" = "0" ] && quit 1
				mkdir -p $baseDir/../delivering/NaviOP
				cp jmobile_portable_simrad.tar.gz $baseDir/../delivering/NaviOP/N-Design_Runtime-v$major.$minor.$sp.$b.tar.gz || quit 1

				fakeroot -- /bin/bash -c "\
					echo k | ./portable_deploy_simrad_discovery_only.sh; \
                        	"
				#[ ! "$?" = "0" ] && quit 1
				cp jmobile_portable_simrad_discovery_only.tar.gz $baseDir/../delivering/NaviOP/N-Design_Runtime-DiscoveryOnly-v$major.$minor.$sp.$b.tar.gz || quit 1
			fi

		elif [ $brand = "openhmi" -a "$CREATE_OPENHMI_PORTABLE" = "true" ]; then
                        cd $baseDir/linuxbuild
                        ./simpledeploypkg_jmobile.sh openhmi || quit 1
                        rm -rf deploy .tmppackage
                        mv jm-devkit.tgz $portableCreatorDir

                        cd $portableCreatorDir
                        git checkout .
                        git checkout master
                        git pull

                        rm -rf mainos
                        fakeroot -- /bin/bash -c "\
                                mkdir mainos; \
                                tar xzf mainos.tar.gz -C mainos; \
                                echo k | ./portable_deploy_devkit.sh; \
                        "
                        [ ! "$?" = "0" ] && quit 1
			mkdir -p $baseDir/../delivering/OpenHMI
			cp jmobile_portable_devkit.tar.gz $baseDir/../delivering/OpenHMI/OpenHMI_Runtime-v$major.$minor.$sp.$b.tar.gz || quit 1

		elif [ $brand = "hetronic" -a "$CREATE_HETRONIC_PORTABLE" = "true" ]; then
                        cd $baseDir/linuxbuild
                        ./simpledeploypkg_jmobile.sh hetronic || quit 1
                        rm -rf deploy .tmppackage
                        mv jm-hetronic.tgz $portableCreatorDir

                        cd $portableCreatorDir
                        git checkout .
                        git checkout master
                        git pull

                        rm -rf mainos
                        fakeroot -- /bin/bash -c "\
                                mkdir mainos; \
                                tar xzf mainos.tar.gz -C mainos; \
                                echo k | ./portable_deploy_hetronic.sh; \
                        "
                        [ ! "$?" = "0" ] && quit 1
			mkdir -p $baseDir/../delivering/hetronic
			cp jmobile_portable_hetronic.tar.gz $baseDir/../delivering/hetronic/MethodeHMI_Runtime-v$major.$minor.$sp.$b.tar.gz || quit 1
                fi
		cd $BuildDir/build;
	   done;
	   popd
	fi

        # Beckup .debug files
        cd $BuildDir;
        [ -d "../debugInfo/${PlatformDir}" ] && rm -rf ../debugInfo/${PlatformDir};
        mkdir -p ../debugInfo/${PlatformDir};

        for f in $( cd build/app ; find . ! -path . ); do
           [ -d build/app/$f ] && mkdir -p ../debugInfo/${PlatformDir}/$f && continue;
           [[ "$f" = *".debug" ]] && cp build/app/$f ../debugInfo/${PlatformDir}/$f;
        done;

        # restore main folder
        popd
done

# just quit if no platform built
[ "$BUILD_FOR_LINUX" = "false" ] && quit 0


# Debug info delivering
GZIP=-9 tar zcvf linuxDebug.tar.gz debugInfo || quit 1
cp -f linuxDebug.tar.gz ../delivering || quit 1
rm linuxDebug.tar.gz;
rm -rf debugInfo;

echo $fver;

echo All done!;
quit 0
