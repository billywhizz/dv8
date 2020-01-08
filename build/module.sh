#!/bin/bash
MODULE_NAME=${1:-os}
CONFIG=${2:-release}
echo "building module $MODULE_NAME ($CONFIG)"
export SRC=../src
export DEPS=../deps
export OUT=../out

export V8_INCLUDE=$DEPS/v8/include
export UV_INCLUDE=$DEPS/uv/include
export ZLIB_INCLUDE=$DEPS/zlib/include
export V8_DEPS=$DEPS/v8
export UV_DEPS=$DEPS/uv
export BUILTINS=$SRC/builtins
export MODULES=$SRC/modules
export MODULE_DIR=$MODULES/$MODULE_NAME
export CC="ccache g++"

if [[ "$CONFIG" == "release" ]]; then
    export CCFLAGS="-I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I$MODULE_DIR -I$ZLIB_INCLUDE -I$SRC -msse4 -fPIC -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -O3 -fno-omit-frame-pointer -fno-rtti -fno-exceptions -std=gnu++1y"
else
    export CCFLAGS="-I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I$MODULE_DIR -I$ZLIB_INCLUDE -I$SRC -msse4 -fPIC -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -g -fno-omit-frame-pointer -fno-rtti -fno-exceptions -std=gnu++1y"
fi
if [[ "$MODULE_NAME" == "httpParser" ]]; then
    export LDFLAGS="-shared -pthread -m64 -Wl,-soname=$MODULE_NAME.so -o ./$MODULE_NAME.so -Wl,--start-group ./http_parser.o ./$MODULE_NAME.o -lz -Wl,--end-group"
    $CC $CCFLAGS -c -o http_parser.o $MODULE_DIR/http_parser.c
else
    export LDFLAGS="-shared -pthread -m64 -Wl,-soname=$MODULE_NAME.so -o ./$MODULE_NAME.so -Wl,--start-group ./$MODULE_NAME.o -lz -Wl,--end-group"
fi
if [[ "$MODULE_NAME" == "picoHttpParser" ]]; then
    export CCFLAGS="-msse4 $CCFLAGS"
fi
# compile the class
$CC $CCFLAGS -c -o $MODULE_NAME.o $MODULE_DIR/$MODULE_NAME.cc
# create the lib
$CC $LDFLAGS
if [[ "$CONFIG" == "release" ]]; then
    strip ./$MODULE_NAME.so
fi
cp -f ./$MODULE_NAME.so $OUT/lib/$MODULE_NAME.so
