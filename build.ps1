$ErrorActionPreference='Stop'

# build the docker image with all the build dependencies
docker build -t webos-build-env:latest . 

# make sure the build directory is clean
docker run -v ${PWD}:/src webos-build-env:latest make clean 

# run the build itself
docker run -v ${PWD}:/src webos-build-env:latest make all
