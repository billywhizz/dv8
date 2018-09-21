export NODE_HOME=/source/node-v$NODE_VERSION
export V8_INCLUDE=$NODE_HOME/deps/v8/include
export UV_INCLUDE=$NODE_HOME/deps/uv/include
export NODE_DEPS=$NODE_HOME/out/Release/obj.target/deps
export PWD=$(pwd)
export BUILTIN_DIR=$(pwd)/builtins
g++ \
    -I$V8_INCLUDE \
    -I$UV_INCLUDE \
    -I./ \
    -I$BUILTIN_DIR \
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
    -o timer.o \
    $BUILTIN_DIR/timer.cc
g++ \
    -I$V8_INCLUDE \
    -I$UV_INCLUDE \
    -I./ \
    -I$BUILTIN_DIR \
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
    -o buffer.o \
    $BUILTIN_DIR/buffer.cc
g++ \
    -I$V8_INCLUDE \
    -I$UV_INCLUDE \
    -I./ \
    -I$BUILTIN_DIR \
    -I/usr/include \
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
    -o isolates.o \
    isolates.cc
# link main executable
g++ \
    -pthread \
    -rdynamic \
    -m64 \
    -pthread \
    -o ./isolates \
    -Wl,--start-group \
    ./timer.o \
    ./buffer.o \
    ./isolates.o \
    $NODE_DEPS/v8/gypfiles/libv8_libplatform.a \
    $NODE_DEPS/v8/gypfiles/libv8_libbase.a \
    $NODE_DEPS/v8/gypfiles/libv8_libsampler.a \
    $NODE_DEPS/v8/gypfiles/libv8_snapshot.a \
    $NODE_DEPS/v8/gypfiles/libv8_base.a \
    $NODE_DEPS/uv/libuv.a \
    -ldl \
    -lrt \
    -lm \
    -lexecinfo \
    -Wl,--end-group
strip ./isolates
rm -f *.o
