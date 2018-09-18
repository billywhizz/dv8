export NODE_HOME=/source/node-v$NODE_VERSION
export V8_INCLUDE=$NODE_HOME/deps/v8/include
export UV_INCLUDE=$NODE_HOME/deps/uv/include
export NODE_DEPS=$NODE_HOME/out/Release/obj.target/deps
export PWD=$(pwd)
export BUILTIN_DIR=$(pwd)/builtins
#rm -f *.o
# compile the builtins
g++ \
    '-DV8_DEPRECATION_WARNINGS=0' \
    '-DHAVE_INSPECTOR=0' \
    '-D__POSIX__' \
    '-D_LARGEFILE_SOURCE' \
    '-D_FILE_OFFSET_BITS=64' \
    '-D_POSIX_C_SOURCE=200112' \
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
    '-DV8_DEPRECATION_WARNINGS=0' \
    '-DHAVE_INSPECTOR=0' \
    '-D__POSIX__' \
    '-D_LARGEFILE_SOURCE' \
    '-D_FILE_OFFSET_BITS=64' \
    '-D_POSIX_C_SOURCE=200112' \
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
# compile the dv8 core library
g++ \
    '-DV8_DEPRECATION_WARNINGS=0' \
    '-DHAVE_INSPECTOR=0' \
    '-D__POSIX__' \
    '-D_LARGEFILE_SOURCE' \
    '-D_FILE_OFFSET_BITS=64' \
    '-D_POSIX_C_SOURCE=200112' \
    -I$V8_INCLUDE \
    -I$UV_INCLUDE \
    -I$BUILTIN_DIR \
    -I./ \
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
    -o dv8.o \
    dv8.cc
# create the lib
ar crsT dv8.a buffer.o timer.o dv8.o
# compile the main executable
g++ \
    '-DV8_DEPRECATION_WARNINGS=0' \
    '-DHAVE_INSPECTOR=0' \
    '-D__POSIX__' \
    '-D_LARGEFILE_SOURCE' \
    '-D_FILE_OFFSET_BITS=64' \
    '-D_POSIX_C_SOURCE=200112' \
    -I$V8_INCLUDE \
    -I$UV_INCLUDE \
    -I$BUILTIN_DIR \
    -I./ \
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
    -o dv8main.o \
    dv8_main.cc
# link main executable
g++ \
    -pthread \
    -rdynamic \
    -m64 \
    -Wl,--whole-archive,$NODE_DEPS/uv/libuv.a \
    -Wl,--no-whole-archive \
    -Wl,-z,noexecstack \
    -Wl,--whole-archive $NODE_DEPS/v8/gypfiles/libv8_base.a \
    -Wl,--no-whole-archive \
    -pthread \
    -o ./dv8 \
    -Wl,--start-group \
    ./dv8main.o \
    ./dv8.a \
    $NODE_DEPS/v8/gypfiles/libv8_libplatform.a \
    $NODE_DEPS/v8/gypfiles/libv8_libbase.a \
    $NODE_DEPS/v8/gypfiles/libv8_libsampler.a \
    $NODE_DEPS/v8/gypfiles/libv8_snapshot.a \
    -ldl \
    -lrt \
    -lm \
    -Wl,--end-group
strip ./dv8
rm -f *.o
