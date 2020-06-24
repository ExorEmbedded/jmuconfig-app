#!/bin/sh

cd $(dirname $0)

cd deploy

PGID=` cat /proc/$$/stat | awk '{print $5}'`
echo $PGID > /var/run/hmi-webkit.pid # save pid for termination
(
    # export required variables (not availables if started by jmlauncher daemon)
    export USER=`busybox whoami`
    export HOME=`eval echo ~$USER`
    ./start.sh $@;
    dbus-send --print-reply --system --dest=com.exor.JMLauncher '/' com.exor.JMLauncher.appFinished string:"Webkit Browser"
    PID="`$(cat /var/run/hmi-webkit.pid)`"
    rm /var/run/hmi-webkit.pid
    pkill -TERM  -s $PID;       # kill all child spawned processes if any
    # notifies jmlauncher daemon we finished
) &

# wait jmobile is started
sleep 5

if ( pgrep WebkitBrowser > /dev/null ); then
        dbus-send --print-reply --system --dest=com.exor.JMLauncher '/' com.exor.JMLauncher.appLoaded string:"Webkit Browser"
fi


