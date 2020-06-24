#!/bin/sh

cd $(dirname $0)
SCRIPTDIR=$(pwd)
echo $SCRIPTDIR

[ -z "$DISPLAY" ] && export DISPLAY=:0

ulimit -c unlimited

DBGLAUNCHER=
DBGLAUNCHEROPTS=
RENICE=

# in fastboot mode runs at increased priority wrt other system services
[ "$FASTBOOT" ] && RENICE="nice -n -10"

HMIARGS=$@

if [ -e .debug ] ; then
	rm /dev/watchdog*
	DBGLAUNCHER="eval gdb"
	DBGLAUNCHEROPTS="-batch -ex \"handle SIGCONT nostop pass\" -ex \"run $@\" -ex generate-core-file -ex quit"
	HMIARGS=
fi


# starts codesys 3 if found
[ -e /mnt/data/hmi/codesys_auto -o -e /mnt/data/hmi/qthmi/codesys_auto ] && [ -e ./rts/codesyscontrol ] && ! pidof codesyscontrol && ( 
cd rts ; 
LD_LIBRARY_PATH=$SCRIPTDIR:.:$LD_LIBRARY_PATH \
PATH=$SCRIPTDIR:$PATH \
QTDIR=$SCRIPTDIR \
QT_PLUGIN_PATH=$SCRIPTDIR/plugin:$SCRIPTDIR:$SCRIPTDIR/imageformats \
$RENICE ./codesyscontrol 
) &

$RENICE  /mnt/data/hmi/un69_server/deploy/start.sh &

if [ "$FASTBOOT" ]; then
	(sleep 60; renice -n 0 $(pidof HMI) ) &
	(sleep 60; renice -n 0 $(pidof codesyscontrol) ) &
fi

LD_LIBRARY_PATH=$SCRIPTDIR:$LD_LIBRARY_PATH \
PATH=$SCRIPTDIR:$PATH \
QTDIR=$SCRIPTDIR \
QT_PLUGIN_PATH=$SCRIPTDIR/plugin:$SCRIPTDIR:$SCRIPTDIR/imageformats \
       $DBGLAUNCHER $DBGLAUNCHEROPTS $RENICE ./HMI $HMIARGS

if ls core* 2> /dev/null ; then
	mkdir -p var/log
	mkdir -p preserved
	mv `find var/log -iname '*crashlog.tar' | sort | tail -n 3` ./preserved
	rm var/log/*crashlog.tar 2> /dev/null 
	mv ./preserved/* var/log 2> /dev/null
	rm -rf preserved
	mkdir -p gdbcoredumps
	mv core* gdbcoredumps
	tar chvzf gdbcoredumps/logs.tar.gz /var/log/
	cp ../../jmlauncher.xml  gdbcoredumps
	cp var/log/*.log gdbcoredumps
	cp var/log/*.txt gdbcoredumps
	tar cvzf var/log/`date +%Y%m%d%H%M%S`-crashlog.tar.gz gdbcoredumps
	rm -rf gdbcoredumps
	sync
fi
