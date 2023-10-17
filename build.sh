#!/bin/bash

# build the docker image with all the build dependencies
docker build -t webos-build-env:latest . || exit 1

# make sure the build directory is clean
docker run -u ${UID} -v ${PWD}:/src webos-build-env:latest make clean || exit 2

# run the build itself
docker run -u ${UID} -v ${PWD}:/src webos-build-env:latest make all || exit 3
