#!/bin/bash
ls /home/autosvn/workspace/jmobile_linux_build/delivering/clang/
CONTAINER_ID="`DOCKER_OPTS="-d -p 8181:8181" ./dr.sh scan-view-8 --host 0.0.0.0 --allow-all-hosts /home/autosvn/workspace/jmobile_linux_build/delivering/clang/2019* | tr -d '\n'`"
echo $CONTAINER_ID
sleep 1
echo -e "Running in background container $CONTAINER_ID \nAttach to log... (Ctr+C to detach)"
docker logs -f "$CONTAINER_ID"
