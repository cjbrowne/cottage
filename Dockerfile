# Used to enshrine the build dependencies in a format that can be easily
# understood by anyone.  If you have all the build deps yourself, or otherwise
# want to build it "natively", then feel free to just run `make` instead of
# building using Docker.  If you want to build using Docker, this file is here.

FROM silkeh/clang:15-bookworm

RUN apt-get update
RUN apt-get -y install xorriso nasm git

WORKDIR /src
