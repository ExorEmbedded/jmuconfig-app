#!/bin/sh
cd $(dirname $0)
cd deploy
file="CPCO/cpco_entries.xml"
if [ -f "$file" ]
then
	echo starting CPCO updates
	#saving old values of tcp window size from file tcp_wmem
	fName="/proc/sys/net/ipv4/tcp_wmem"
	oldvalues=`cat $fName`
	#enters new small values
	echo '4096 4096 4096' > $fName	
	sh -c './HTTPUpdateUtility 2> ./CPCO/cpco.log'
	#restoring old values
	echo $oldvalues> $fName
fi
dbus-send --print-reply --system --dest=com.exor.JMLauncher '/' com.exor.JMLauncher.installFinished int32:0 string:""

