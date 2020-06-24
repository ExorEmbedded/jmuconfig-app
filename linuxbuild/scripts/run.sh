#!/bin/sh

cd $(dirname $0)


cd deploy

[ -e /etc/default/rcS ] && source /etc/default/rcS
export FASTBOOT

PGID=` cat /proc/$$/stat | awk '{print $5}'`
echo $PGID > /var/run/hmi-qthmi.pid # save pid for termination
(
    # export required variables (not availables if started by jmlauncher daemon)
    export USER=`busybox whoami`
    export HOME=`eval echo ~$USER`
    echo "Starting HMI as user \"$USER\" with home \"$HOME\""
    ./start.sh $@;
    if [ -z "$FASTBOOT" ] && (pidof jmlauncher &>/dev/null); then
        dbus-send --print-reply --system --dest=com.exor.JMLauncher '/' com.exor.JMLauncher.appFinished string:"HMI Runtime"
    fi
    PID="$(cat /var/run/hmi-qthmi.pid)"
    rm /var/run/hmi-qthmi.pid
    pkill -TERM  -s $PID;       # kill all child spawned processes if any
) &

# wait HMI Runtime is started
sleep 5

if ( pgrep HMI > /dev/null ); then
    # starts debugging resources (print a report every 60 minutes and check ram usage every 60 seconds)
    dbus-send --print-reply --system --dest=com.exor.EPAD /SysStat com.exor.EPAD.SysStat.monitorSystemResources int32:60 int32:60
    if [ -z "$FASTBOOT" ]; then
        dbus-send --print-reply --system --dest=com.exor.JMLauncher '/' com.exor.JMLauncher.appLoaded string:"HMI Runtime"
    fi
fi

