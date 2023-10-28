PROJECT_ROOT=$(pwd)

function create_module()
{
    mkdir -p $PROJECT_ROOT/kernel/src/$1
    touch $PROJECT_ROOT/kernel/src/$1/$1.c
    mkdir -p $PROJECT_ROOT/kernel/include/$1
    touch $PROJECT_ROOT/kernel/include/$1/$1.h
}



