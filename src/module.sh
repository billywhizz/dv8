echo building module $MODULE_NAME
export NODE_HOME=/source/node-v$NODE_VERSION
export V8_INCLUDE=$NODE_HOME/deps/v8/include
export UV_INCLUDE=$NODE_HOME/deps/uv/include
export NODE_DEPS=$NODE_HOME/out/Release/obj.target/deps
export PWD=$(pwd)
export MODULE_DIR=$(pwd)/modules/$MODULE_NAME
# compile the binding
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
    -I$MODULE_DIR \
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
    '-DV8_DEPRECATION_WARNINGS=0' \
    '-DHAVE_INSPECTOR=0' \
    '-D__POSIX__' \
    '-D_LARGEFILE_SOURCE' \
    '-D_FILE_OFFSET_BITS=64' \
    '-D_POSIX_C_SOURCE=200112' \
    -I$V8_INCLUDE \
    -I$UV_INCLUDE \
    -I./ \
    -I$MODULE_DIR \
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
#ar crsT $MODULE_NAME.a $MODULE_NAME.binding.o $MODULE_NAME.o
# link library
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
rm -f *.o