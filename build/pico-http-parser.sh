#!/bin/sh
MODULE_NAME=picoHttpParser
CONFIG=${2:-release}
echo building module $MODULE_NAME
export V8_INCLUDE=/deps/v8/include
export UV_INCLUDE=/deps/uv/include
export V8_DEPS=/deps/v8/out.gn/x64.release/obj
export UV_DEPS=/deps/uv/out/Release
export BUILTINS=/src/builtins
export MODULE_DIR=/src/modules/$MODULE_NAME
export SOCKET_DIR=/src/modules/socket

if [[ "$CONFIG" == "release" ]]; then
    export CCFLAGS="-I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I$MODULE_DIR -I$SOCKET_DIR -I/src -msse4 -fPIC -pthread -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -O3 -fno-omit-frame-pointer -fno-rtti -fno-exceptions -std=gnu++1y"
else
    export CCFLAGS="-I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I$MODULE_DIR -I$SOCKET_DIR -I/src -msse4 -fPIC -pthread -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -g -fno-omit-frame-pointer -fno-rtti -fno-exceptions -std=gnu++1y"
fi
export LDFLAGS="-shared -pthread -msse4 -m64 -Wl,-soname=$MODULE_NAME.so -o ./$MODULE_NAME.so -Wl,--start-group ./$MODULE_NAME.o ./$MODULE_NAME.binding.o -Wl,--end-group"

# compile the binding
g++ $CCFLAGS -c -o $MODULE_NAME.binding.o $MODULE_DIR/binding.cc
# compile the class
g++ $CCFLAGS -c -o $MODULE_NAME.o $MODULE_DIR/$MODULE_NAME.cc
# create the lib
g++ $LDFLAGS
if [[ "$CONFIG" == "release" ]]; then
    strip ./$MODULE_NAME.so
fi
cp -f ./$MODULE_NAME.so /out/lib/$MODULE_NAME.so
