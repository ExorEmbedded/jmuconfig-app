#!/bin/bash

docker run --rm -ti $DOCKER_OPTS  -v /opt:/opt  -v /home:/home -v /etc/passwd:/etc/passwd -u $(id -u):$(id -g) jmobile-build-env $@

