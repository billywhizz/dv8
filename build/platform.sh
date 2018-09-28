#!/bin/sh
echo building dv8 platform
export V8_INCLUDE=/deps/v8/include
export UV_INCLUDE=/deps/uv/include
export V8_DEPS=/deps/v8/out.gn/x64.release/obj
export UV_DEPS=/deps/uv/out/Release
export BUILTINS=/src/builtins
# compile the builtins
g++ \
    -I$V8_INCLUDE \
    -I$UV_INCLUDE \
    -I$BUILTINS \
    -I/src \
    -pthread \
    -Wall \
    -Wextra \
    -Wno-unused-parameter \
    -Wno-unused-variable \
    -Wno-unused-function \
    -m64 \
    -O3 \
    -fno-omit-frame-pointer \
    -fno-rtti \
    -fno-exceptions \
    -std=gnu++1y \
    -c \
    -o tty.o \
    /src/builtins/tty.cc
g++ \
    -I$V8_INCLUDE \
    -I$UV_INCLUDE \
    -I$BUILTINS \
    -I/src \
    -pthread \
    -Wall \
    -Wextra \
    -Wno-unused-parameter \
    -Wno-unused-variable \
    -Wno-unused-function \
    -m64 \
    -O3 \
    -fno-omit-frame-pointer \
    -fno-rtti \
    -fno-exceptions \
    -std=gnu++1y \
    -c \
    -o buffer.o \
    /src/builtins/buffer.cc
g++ \
    -I$V8_INCLUDE \
    -I$UV_INCLUDE \
    -I$BUILTINS \
    -I/src \
    -pthread \
    -Wall \
    -Wextra \
    -Wno-unused-parameter \
    -Wno-unused-variable \
    -Wno-unused-function \
    -m64 \
    -O3 \
    -fno-omit-frame-pointer \
    -fno-rtti \
    -fno-exceptions \
    -std=gnu++1y \
    -c \
    -o timer.o \
    /src/builtins/timer.cc
# compile the dv8 core library
g++ \
    -I$V8_INCLUDE \
    -I$UV_INCLUDE \
    -I$BUILTINS \
    -I/src \
    -pthread \
    -Wall \
    -Wextra \
    -Wno-unused-parameter \
    -Wno-unused-variable \
    -Wno-unused-function \
    -m64 \
    -O3 \
    -fno-omit-frame-pointer \
    -fno-rtti \
    -fno-exceptions \
    -std=gnu++1y \
    -c \
    -o dv8.o \
    /src/dv8.cc
# create the lib
rm -f dv8.a
ar crsT dv8.a buffer.o timer.o tty.o dv8.o
# compile the main executable
g++ \
    -I$V8_INCLUDE \
    -I$UV_INCLUDE \
    -I$BUILTINS \
    -I/src \
    -pthread \
    -Wall \
    -Wextra \
    -Wno-unused-parameter \
    -Wno-unused-variable \
    -Wno-unused-function \
    -m64 \
    -O3 \
    -fno-omit-frame-pointer \
    -fno-rtti \
    -fno-exceptions \
    -std=gnu++1y \
    -c \
    -o dv8main.o \
    /src/dv8_main.cc
g++ \
    -I$UV_INCLUDE \
    -I/src \
    -pthread \
    -Wall \
    -Wextra \
    -Wno-unused-parameter \
    -Wno-unused-variable \
    -Wno-unused-function \
    -m64 \
    -O3 \
    -fno-omit-frame-pointer \
    -fno-rtti \
    -fno-exceptions \
    -std=gnu++1y \
    -c \
    -o ttyTest.o \
    /src/tty-test.cc
# link main executable
g++ \
    -pthread \
    -rdynamic \
    -m64 \
    -o ./dv8 \
    -Wl,--start-group \
    ./dv8main.o \
    ./dv8.a \
    $V8_DEPS/libv8_monolith.a \
    $UV_DEPS/libuv.a \
    -ldl \
    -lrt \
    -lm \
    -Wl,--end-group
strip ./dv8
g++ \
    -pthread \
    -rdynamic \
    -m64 \
    -o ./count \
    -Wl,--start-group \
    ./ttyTest.o \
    $UV_DEPS/libuv.a \
    -ldl \
    -lrt \
    -lm \
    -Wl,--end-group
strip ./count
