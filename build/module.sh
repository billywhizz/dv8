#!/bin/sh
MODULE_NAME=${1:-os}
echo building module $MODULE_NAME
export V8_INCLUDE=/deps/v8/include
export UV_INCLUDE=/deps/uv/include
export V8_DEPS=/deps/v8/out.gn/x64.release/obj
export UV_DEPS=/deps/uv/out/Release
export BUILTINS=/src/builtins
export MODULE_DIR=/src/modules/$MODULE_NAME
# compile the binding
g++ \
    -I$V8_INCLUDE \
    -I$UV_INCLUDE \
    -I$BUILTINS \
    -I$MODULE_DIR \
    -I/src \
    -fPIC \
    -pthread \
    -Wall \
    -Wextra \
    -Wno-unused-parameter \
    -m64 \
    -O3 \
    -fno-omit-frame-pointer \
    -fno-rtti \
    -fno-exceptions \
    -std=gnu++1y \
    -c \
    -o $MODULE_NAME.binding.o \
    $MODULE_DIR/binding.cc
# compile the class
g++ \
    -I$V8_INCLUDE \
    -I$UV_INCLUDE \
    -I$BUILTINS \
    -I$MODULE_DIR \
    -I/src \
    -fPIC \
    -pthread \
    -Wall \
    -Wextra \
    -Wno-unused-parameter \
    -m64 \
    -O3 \
    -fno-omit-frame-pointer \
    -fno-rtti \
    -fno-exceptions \
    -std=gnu++1y \
    -c \
    -o $MODULE_NAME.o \
    $MODULE_DIR/$MODULE_NAME.cc
# create the lib
g++ \
    -shared \
    -pthread \
    -rdynamic \
    -m64 \
    -Wl,-soname=$MODULE_NAME.so \
    -o ./$MODULE_NAME.so \
    -Wl,--start-group \
    ./$MODULE_NAME.o \
    ./$MODULE_NAME.binding.o \
    -Wl,--end-group
strip ./$MODULE_NAME.so
