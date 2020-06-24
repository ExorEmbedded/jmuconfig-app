#!/bin/sh

EXITCODE_LOWMEM=10

ARGS="$@"

# Manage --run-once flag. Ensure only one instance of the application
# is running using this flag
for arg in $ARGS; do
        if [ "$arg" == "--run-once" ]; then
                ARGS=$( echo "$ARGS" | sed 's/--run-once//')
                [ -e /var/run/single-webkit.pid ] && kill -TERM -- -$( cat /var/run/single-webkit.pid ) &> /dev/null
                PGID=` cat /proc/$$/stat | awk '{print $5}'`
                echo $PGID > /var/run/single-webkit.pid
                break
        fi
done

cd $(dirname $0)
SCRIPTDIR=$(pwd)
echo $SCRIPTDIR

export DISPLAY=:0

# reset backlight info to initial state
rm -f /tmp/hmibrowser_backlight_off 

while true; do
	
	LD_LIBRARY_PATH=$SCRIPTDIR:$LD_LIBRARY_PATH \
	PATH=$SCRIPTDIR:$PATH \
	QTDIR=$SCRIPTDIR \
	QT_PLUGIN_PATH=$SCRIPTDIR\plugin:$SCRIPTDIR:$SCRIPTDIR\imageformats \
	       ./WebkitBrowser -f $ARGS
	
	[ $? -eq ${EXITCODE_LOWMEM} ] || break;

	echo "WebkitBrowser returned ${EXITCODE_LOWMEM} (EXITCODE_LOWMEM) - respawning"

done
