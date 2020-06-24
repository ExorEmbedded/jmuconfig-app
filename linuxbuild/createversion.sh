#!/bin/bash
cd `dirname $0`
OUTPUTFOLDER=$1
VERSIONXML=$OUTPUTFOLDER/hmiversion.xml
echo "<files>" > $VERSIONXML
echo "<executablesVersion>23</executablesVersion>" >> $VERSIONXML
createEntry()
{
   if [[ '-f' == $2 ]] ; then
		for f in $1*; do
			if [[ -f $f ]]; then
				cs=$(cksum $f)
				csum=${cs%% *}
				path=${f##*$OUTPUTFOLDER/}
				#echo ${f##*$OUTPUTFOLDER} ${cs%% *}
				echo "<file version=\"\" checksum=\"$csum\" nameTo=\"$path\" nameFrom=\"$path\"/>" >> $VERSIONXML
			fi
		done
	fi
	if [[ '-r' == $2 ]] ; then
		for f in $(find $1); do
			if [[ -f $f ]]; then
			  	cs=$(cksum $f)
				csum=${cs%% *}
				path=${f##*$OUTPUTFOLDER/}
				#echo ${f##*$OUTPUTFOLDER} ${cs%% *}
				echo "<file version=\"\" checksum=\"$csum\" nameTo=\"$path\" nameFrom=\"$path\"/>" >> $VERSIONXML
		  	fi
		done
	fi
}
echo "<group name=\"Executable files\">" >> $VERSIONXML
createEntry $OUTPUTFOLDER/ -f
echo "</group>" >> $VERSIONXML
echo "<group name=\"Console files\">" >> $VERSIONXML
createEntry $OUTPUTFOLDER/workspace -r
echo "</group>" >> $VERSIONXML
echo "<group name=\"Support files\">" >> $VERSIONXML
for searchFolder in config templates actions images imageformats protocols; do
    [ -e "$searchFolder" ] || continue
    createEntry $OUTPUTFOLDER/$searchFolder -r
done
echo "</group>" >> $VERSIONXML
echo "</files>" >> $VERSIONXML

