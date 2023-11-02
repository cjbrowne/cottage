# usage: `source useful-aliases.sh`
# a collection of stuff that's useful inside this project
# to make the DX a bit more pleasant.

PROJECT_ROOT=$(pwd)

function create_module()
{
    mkdir -p $PROJECT_ROOT/kernel/src/$1
    touch $PROJECT_ROOT/kernel/src/$1/$1.c
    mkdir -p $PROJECT_ROOT/kernel/include/$1
    touch $PROJECT_ROOT/kernel/include/$1/$1.h
}



