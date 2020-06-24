colorstart='\e[1;36m' # Black - Regular
redstart='\e[1;31m'
greenstart='\e[1;32m'
yellowstart='\e[1;33m'
graystart='\e[0;37m' # light gray
cyanstart='\e[0;36m' # light cyan
colorend='\e[0m'    # Text Reset

CDS3_DIR="../qthmi/Support/install/files/common/codesysv3-v3.5.12"

if [ $1 ] ; then
    echo "     Deploying package for target " $1
    echo "======================================================"
    if [ $1 == "pc" ] ; then
        PLATFORM="Linux/x86-32"
        JM_PLATFORM="LIN32"
        OUTPUTFOLDER="deploy/x86/"
        BUILDPATH='../build-linx11-x86-release/'$BUILD_SUBDIR
        PACKAGENAME=$PKG_PREFIX'-pc.tgz'
        JMPACKAGENAME=$PKG_PREFIX'-pc.zip'
	HMIABOUTDIR='.'
    elif [ $1 == "pcdbg" ] ; then
        PLATFORM="Linux/x86-32"
        JM_PLATFORM="LIN32"
        OUTPUTFOLDER="deploy/x86/"
        BUILDPATH='../build-linx11-x86-debug/'$BUILD_SUBDIR
        PACKAGENAME=$PKG_PREFIX'-pcdbg.tgz'
        JMPACKAGENAME=$PKG_PREFIX'-pcdbg.zip'
	HMIABOUTDIR='.'
    elif [ $1 == "altera" ] ; then
        PLATFORM="Linux OE/altera"
        JM_PLATFORM="UN60_LinuxOE"
        OUTPUTFOLDER="deploy/altera/"
        BUILDPATH='../build-linx11-usom01-release/'$BUILD_SUBDIR
        PACKAGENAME=$PKG_PREFIX'-altera.tgz'
        JMPACKAGENAME=$PKG_PREFIX'-altera.zip'
	HMIABOUTDIR='altera'
    elif [ $1 == "hetronic" ] ; then
        PLATFORM="Linux OE/hetronic"
        JM_PLATFORM="UN60_LinuxOE"
        OUTPUTFOLDER="deploy/hetronic/"
        BUILDPATH='../build-linx11-usom01-release/'$BUILD_SUBDIR
        PACKAGENAME=$PKG_PREFIX'-hetronic.tgz'
        JMPACKAGENAME=$PKG_PREFIX'-hetronic.zip'
	HMIABOUTDIR='hetronic'
    elif [ $1 == "usom01" ] ; then
        PLATFORM="Linux OE/usom01"
        JM_PLATFORM="UN60_LinuxOE"
        OUTPUTFOLDER="deploy/usom01/"
        BUILDPATH='../build-linx11-usom01-release/'$BUILD_SUBDIR
        PACKAGENAME=$PKG_PREFIX'-usom01.tgz'
        JMPACKAGENAME=$PKG_PREFIX'-usom01.zip'
	HMIABOUTDIR='.'
    elif [ $1 == "usom01-portable" ] ; then
        PLATFORM="Linux OE/usom01-portable"
        JM_PLATFORM="UN60_LinuxOE"
        OUTPUTFOLDER="deploy/usom01-portable/"
        BUILDPATH='../build-linx11-usom01-release/'$BUILD_SUBDIR
        PACKAGENAME=$PKG_PREFIX'-usom01-portable.tgz'
        JMPACKAGENAME=$PKG_PREFIX'-usom01-portable.zip'
	HMIABOUTDIR='.'
    elif [ $1 == "simrad" ] ; then
        PLATFORM="Linux OE/simrad"
        JM_PLATFORM="UN60_LinuxOE"
        OUTPUTFOLDER="deploy/simrad/"
        BUILDPATH='../build-linx11-usom01-release/'$BUILD_SUBDIR
        PACKAGENAME=$PKG_PREFIX'-simrad.tgz'
        JMPACKAGENAME=$PKG_PREFIX'-simrad.zip'
	HMIABOUTDIR='naviop'
    elif [ $1 == "simrad-qt5" ] ; then
        PLATFORM="Linux OE/simrad"
        JM_PLATFORM="UN60_QT5_LinuxOE"
        OUTPUTFOLDER="deploy/simrad/"
        BUILDPATH='../build5-linx11-usom01-release/'$BUILD_SUBDIR
        PACKAGENAME=$PKG_PREFIX'-simrad-qt5.tgz'
        JMPACKAGENAME=$PKG_PREFIX'-simrad-qt5.zip'
	HMIABOUTDIR='naviop'
    elif [ $1 == "openhmi" ] ; then
        PLATFORM="Linux OE/devkit"
        JM_PLATFORM="UN60_LinuxOE"
        OUTPUTFOLDER="deploy/devkit/"
        BUILDPATH='../build-linx11-usom01-release/'$BUILD_SUBDIR
        PACKAGENAME=$PKG_PREFIX'-devkit.tgz'
        JMPACKAGENAME=$PKG_PREFIX'-devkit.zip'
	HMIABOUTDIR='openhmi'
    else
    echo "Unknown target $1";
    exit;
  fi
else
  echo "Specify a target (altera, usom01, usom01-portable, hetronic, pc, pcdbg)"
  echo "E.g."
  echo ""
  echo "simpledeploypkg.sh usom01 zip"
  echo ""
  echo "Generates a deploy pkg."
  exit
fi

if [ -e $BUILDPATH ] ;  then
	echo "Found build path $BUILDPATH"
fi

echo "Going to remove $OUTPUTFOLDER. Press any key to continue"
read -r line
rm -rf $OUTPUTFOLDER || exit 1

echo "Saving output to folder $OUTPUTFOLDER"
mkdir -p $OUTPUTFOLDER || exit 1

# add binary deps
addDeps()
{
    HMIRPATH=$(readelf -d $1 | grep RPATH | awk 'BEGIN { FS="[\\[\\]]" } { print $2}' | tr ":" "\n")
    HMIDEPS=$(readelf -d $1 | grep NEEDED | awk 'BEGIN { FS="[\\[\\]]" } { print $2}')
    for dep in $HMIDEPS ; do
        for folder in $HMIRPATH ; do
            if [[ $folder =~ ^/.* ]] ; then
                if [[ -e $folder/$dep ]] ; then
                    if [[ ! -e $OUTPUTFOLDER/$dep ]]; then
                        resolvedDep=$(readlink -f $folder/$dep)
                        echo -e $graystart "Adding dep " $resolvedDep " as " $dep " from $folder" $colorend
                        cp $resolvedDep $OUTPUTFOLDER/$dep
                        chmod 0777 $OUTPUTFOLDER/$dep
                    fi
                fi
            fi
        done
    done
}

# copy binary
copyBin()
{
        echo -e $cyanstart "Add binary " $1 $colorend
        cp $1 $OUTPUTFOLDER
        chmod 0777 $1
        addDeps $1
}

toLower()
{
    NEWFILE=`echo ${1,,}`
    if [[ $1 != $NEWFILE ]] ; then
        echo -e $yellowstart "convert $1 to lower case " $colorstop
        mv $1 $NEWFILE
    fi
}

lowerCaseRecurse()
{
    for f in `find $1 -maxdepth 1 ` ; do
        toLower $f
        if [[ -d $f ]] ; then
            if [[ $f != $1 ]] ; then
                # recurse lowercase folder
                lowerCaseRecurse ${f,,}
            fi
        fi
    done
}

copyQtPlugins()
{
    # copy qt plugins
    QTPLUGINS=$( strings $OUTPUTFOLDER/libQt*Core* | grep -i qt_plugpath | awk 'BEGIN { FS="=" } { print $2}' )
    echo $QTPLUGINS
    mkdir -p $OUTPUTFOLDER/imageformats
    cp -a $QTPLUGINS/imageformats/*.so $OUTPUTFOLDER/imageformats/
}

createPkg()
{
    echo -e $greenstart "Creating compressed package $PACKAGENAME" $colorend

    rm $PACKAGENAME 2> /dev/null
    rm $JMPACKAGENAME 2> /dev/null
    rm -rf  .tmppackage 2> /dev/null
    mkdir -p .tmppackage/deploy
    cp -a $OUTPUTFOLDER/* .tmppackage/deploy/

    cp $1/run.sh .tmppackage/
    chmod 0777 .tmppackage/run.sh

    cp $1/stop.sh .tmppackage/
    chmod 0777 .tmppackage/stop.sh

    cp $1/package.info .tmppackage/
    if [ $HMIVERSION ]; then
            echo "Setting current HMI version in package.info to $HMIVERSION"
            sed -i "s:<version.*$:<version>$HMIVERSION</version>:" .tmppackage/package.info
            sed -i "s:<name.*$:<name>HMI Runtime</name>:" .tmppackage/package.info
    fi

    cp $1/install.sh .tmppackage/
    chmod 0777 .tmppackage/install.sh

    cp $1/uninstall.sh .tmppackage/
    chmod 0777 .tmppackage/uninstall.sh

    cp $1/update.sh .tmppackage/
    chmod 0777 .tmppackage/update.sh

    cd .tmppackage

    if [[ $2 == "zip" ]] ; then
        echo -e $yellowstart CREATING ZIP PACKAGE FOR JMLAUNCHER $colorend

        zip -9 -y -r -q -P "q3E43GcoffqQAAafpfkDaFwDnEMWVBX3cjGTz5YESXfCjUE3uiBatszarMdhjyVV" ../$JMPACKAGENAME *
    else
        tar czf ../$PACKAGENAME --transform "s,\./,," --show-transformed .
        ls -lh ../$PACKAGENAME
    fi
    cd ..
}

