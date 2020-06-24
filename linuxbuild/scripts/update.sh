#!/bin/sh

cd $(dirname $0)
cd deploy
./HMIUpdater -d $1 -s $2
rc=$?
echo  result is $rc

file="CPCO/cpco_entries.xml"
if [ -f "$file" ]
then
	echo starting CPCO updates
	#saving old values of tcp window size from file tcp_wmem
	fName="/proc/sys/net/ipv4/tcp_wmem"
	oldvalues=`cat $fName`
	#enters new small values
	echo '4096 4096 4096' > $fName
	export DISPLAY=:0 	
	sh -c './HTTPUpdateUtility 2> ./CPCO/cpco.log'
	#restoring old values
	echo $oldvalues> $fName
fi

if [ $rc -eq 0 ] ; then
dbus-send --print-reply --system --dest=com.exor.JMLauncher '/' com.exor.JMLauncher.updateFinished int32:0 string:""
else
dbus-send --print-reply --system --dest=com.exor.JMLauncher '/' com.exor.JMLauncher.updateFinished int32:1 string:"Failed to copy files"
fi





