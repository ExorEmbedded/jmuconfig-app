#!/bin/bash

cd $(dirname $0)

. ./common.sh

git clone git@bitbucket.exorint.cloud:jm/jmdocker.git

pushd .

cd jmdocker
git fetch --all
git reset --hard origin/master
cp ../jm-pc.tgz files/ || quit 1;
cp ../epad-pc.tgz files/ || quit 1;
cp ../jml-pc.tgz files/ || quit 1;
docker build -t jm-docker:$major.$minor.$sp.$b . || quit 1;
docker tag jm-docker:$major.$minor.$sp.$b jm-docker:$major.$minor || quit 1
docker tag jm-docker:$major.$minor.$sp.$b jm-docker:latest || quit 1
docker save jm-docker:$major.$minor.$sp.$b jm-docker:$major.$minor jm-docker:latest > ../jmdocker.tar || quit 1;
docker rmi `docker images | grep jm-docker | awk '{print $3}'`

popd

