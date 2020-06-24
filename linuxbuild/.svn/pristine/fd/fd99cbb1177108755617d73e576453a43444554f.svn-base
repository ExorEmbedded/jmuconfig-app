#!/bin/sh

cd $(dirname $0)
export SCRIPTDIR=$(pwd)
echo $SCRIPTDIR

[ -z "$DISPLAY" ] && export DISPLAY=:0
[ -e /tmp/.X0-lock ] && export QT_QPA_PLATFORM=xcb
ulimit -c unlimited

DBGLAUNCHER=
DBGLAUNCHEROPTS=
RENICE=

# in fastboot mode runs at increased priority wrt other system services
[ "$FASTBOOT" ] && export RENICE="nice -n -10"

HMIARGS=$@

if [ -e .debug ] ; then
	rm /dev/watchdog*
	DBGLAUNCHER="eval gdb"
	DBGLAUNCHEROPTS="-batch -ex \"handle SIGCONT nostop pass\" -ex \"run $@\" -ex generate-core-file -ex quit"
	HMIARGS=
fi

# Start PLC software
( ! pidof codesyscontrol ) && ( ! pidof LLExec ) && (

. /etc/exorint.funcs

if [ -e ./rts/codesyscontrol ] && (
	[ -e /mnt/data/hmi/codesys_auto ] ||
	[ -e /mnt/data/hmi/qthmi/codesys_auto ] ||
	[ "$( exorint_swflagarea_bit 1 )" -eq 1 ] ); then

	echo Starting Codesys 3
	cd rts
	if [ -e "/mnt/data/hmi/restorecdsdefault.bin" ]; then
		rm -rf PlcLogic
		[ -e "/dev/fram" ] && dd if=/dev/zero of=/dev/fram bs=1024 count=64
		rm -rf /mnt/data/hmi/restorecdsdefault.bin
		sync
	fi

	while [ "$( pidof codesyscontrol )" == "" ]; do
		LD_LIBRARY_PATH=$SCRIPTDIR:.:$LD_LIBRARY_PATH \
		PATH=$SCRIPTDIR:/bin:/usr/bin:/sbin:/usr/sbin \
		QTDIR=$SCRIPTDIR \
		QT_PLUGIN_PATH=$SCRIPTDIR/plugin:$SCRIPTDIR:$SCRIPTDIR/imageformats \
		$RENICE ./codesyscontrol
		sleep 1;
	done
elif [ -e ./xplc/LLExec ] && (
	[ -e /mnt/data/hmi/qthmi/xplc_auto ] ||
	[ "$( exorint_swflagarea_bit 18 )" -eq 1 ] ); then

	echo Starting XPLC
	./xplc/start.sh
fi

)&

# Blink feedback
[ "$FASTBOOT" ] && [ "$(exorint_ver_bsp)" = "UN67" ] && ( 
	sleep 5
	chrt --all-tasks --fifo -p 30 $(pidof HMI)
) &

[ "$FASTBOOT" ] && (
	sleep 60
	renice -n 0 $(pidof HMI)
	renice -n 0 $(pidof codesyscontrol)
	renice -n 0 $(pidof LLExec)
) &

[ -e "$SCRIPTDIR/plugin/platforminputcontexts/libdbusvirtualkeyboardplugin.so" ] && IM_MODULE="dbusvirtualkeyboard"

# reduce stack size
ulimit -s 1024
QT_IM_MODULE=$IM_MODULE \
LD_LIBRARY_PATH=$SCRIPTDIR:$SCRIPTDIR/protocols:$LD_LIBRARY_PATH \
PATH=$SCRIPTDIR:$PATH \
QTDIR=$SCRIPTDIR \
QT_PLUGIN_PATH=$SCRIPTDIR/plugin:$SCRIPTDIR:$SCRIPTDIR/imageformats \
       $DBGLAUNCHER $DBGLAUNCHEROPTS $RENICE ./HMI $HMIARGS

if ls core* 2> /dev/null ; then
	mkdir -p var/log
	mkdir -p preserved
	mv `find var/log -iname '*crashlog.tar.gz' | sort | tail -n 3` ./preserved
	rm var/log/*crashlog.tar.gz 2> /dev/null
	mv ./preserved/* var/log 2> /dev/null
	rm -rf preserved
	mkdir -p gdbcoredumps
	mv core* gdbcoredumps
	tar chvzf gdbcoredumps/logs.tar.gz /var/log/
	cp ../../jmlauncher.xml  gdbcoredumps
	cp var/log/*.log gdbcoredumps
	cp var/log/*.txt gdbcoredumps
	CRASHLOGNAME="`date +%Y%m%d%H%M%S`-crashlog.tar.gz"
	REPORTNAME="gdbcoredumps/crashreport.log"
	cat << EOF > $REPORTNAME
HMI encountered a fatal error:

UTC time    : `date -u`
HMI version : `strings ./HMI | grep "\- Build (" `
Protocols   : $(cd protocols; for file in `find -iname "*.so"` ; do strings $file | grep 'Version=[0-9]\+' | sed 's,.*>Name=\([^?]\+\).*Version=\([0-9]\+\).*,\1:\2,' ; done )
HMI Project : `cat config/system.xml | grep projectName | sed 's,\(.*<projectName>\)\|\(</projectName>\)\|\(.*<projectName/>\),,g'`
BSP version : `cat /boot/version `
System      : `uname -a `
Uptime      : `cat /proc/uptime`
Load        : `cat /proc/loadavg`
Log file    :  $(pwd)/var/log/$CRASHLOGNAME

`ps aux`
EOF
	tar cvzf var/log/$CRASHLOGNAME gdbcoredumps
	./HMIWdDialog -c -n HMI -r $(pwd)/var/log/$CRASHLOGNAME -p HMI -d "`cat $REPORTNAME`"
	rm -rf gdbcoredumps
	sync
fi
