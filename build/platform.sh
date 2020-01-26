#!/bin/bash
CONFIG=${1:-release}
echo "building dv8 platform ($CONFIG)"
export DV8_DEPS=/deps
export DV8_SRC=/src
export DV8_OUT=/build
export V8_INCLUDE=$DV8_DEPS/v8/include
export UV_INCLUDE=$DV8_DEPS/uv/include
export V8_DEPS=$DV8_DEPS/v8
export UV_DEPS=$DV8_DEPS/uv
export BUILTINS=$DV8_SRC/builtins
export SSL_PREFIX=/usr/lib/x86_64-linux-gnu
export TRACE="TRACE=0"
export STATIC="DV8STATIC=1"

if [[ "$CONFIG" == "release" ]]; then
    export CCFLAGS="-D$STATIC -D$TRACE -I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I$DV8_SRC -msse4 -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -O3 -fno-omit-frame-pointer -fno-rtti -ffast-math -fno-ident -fno-exceptions -fmerge-all-constants -fno-unroll-loops -fno-unwind-tables -fno-math-errno -fno-stack-protector -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -std=gnu++1y"
    export LDFLAGS="-pthread -m64 -Wl,-z,norelro -Wl,--start-group $SSL_PREFIX/libssl.a $SSL_PREFIX/libcrypto.a $DV8_OUT/dv8main.o $DV8_OUT/dv8.a $V8_DEPS/libv8_monolith.a $UV_DEPS/libuv.a -lz -ldl -Wl,--end-group"
else
    export CCFLAGS="-D$STATIC -D$TRACE -I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I$DV8_SRC -msse4 -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -g -fno-omit-frame-pointer -fno-rtti -fno-exceptions -std=gnu++1y"
    export LDFLAGS="-pthread -m64 -Wl,-z,norelro -Wl,--start-group $SSL_PREFIX/libssl.a $SSL_PREFIX/libcrypto.a $DV8_OUT/dv8main.o $DV8_OUT/dv8.a $V8_DEPS/libv8_monolith.a $UV_DEPS/libuv.a -lz -ldl -Wl,--end-group"
fi
export CC="ccache g++"

# compile the builtins
$CC $CCFLAGS -c -o $DV8_OUT/buffer.o $DV8_SRC/builtins/buffer.cc
$CC $CCFLAGS -c -o $DV8_OUT/env.o $DV8_SRC/builtins/env.cc

# compile the core modules
$CC $CCFLAGS -I$DV8_SRC/modules/process -c -o $DV8_OUT/process.o $DV8_SRC/modules/process/process.cc
$CC $CCFLAGS -I$DV8_SRC/modules/thread -c -o $DV8_OUT/thread.o $DV8_SRC/modules/thread/thread.cc
$CC $CCFLAGS -I$DV8_SRC/modules/loop -c -o $DV8_OUT/loop.o $DV8_SRC/modules/loop/loop.cc
$CC $CCFLAGS -I$DV8_SRC/modules/timer -c -o $DV8_OUT/timer.o $DV8_SRC/modules/timer/timer.cc
$CC $CCFLAGS -I$DV8_SRC/modules/socket -c -o $DV8_OUT/socket.o $DV8_SRC/modules/socket/socket.cc
$CC $CCFLAGS -I$DV8_SRC/modules/udp -c -o $DV8_OUT/udp.o $DV8_SRC/modules/udp/udp.cc
$CC $CCFLAGS -I$DV8_SRC/modules/tty -c -o $DV8_OUT/tty.o $DV8_SRC/modules/tty/tty.cc
$CC $CCFLAGS -I$DV8_SRC/modules/fs -c -o $DV8_OUT/fs.o $DV8_SRC/modules/fs/fs.cc
$CC $CCFLAGS -I$DV8_SRC/modules/os -c -o $DV8_OUT/os.o $DV8_SRC/modules/os/os.cc
$CC $CCFLAGS -I$DV8_SRC/modules/libz -c -o $DV8_OUT/libz.o $DV8_SRC/modules/libz/libz.cc
$CC $CCFLAGS -I$DV8_SRC/modules/openssl -c -o $DV8_OUT/openssl.o $DV8_SRC/modules/openssl/openssl.cc

$CC $CCFLAGS -c -o $DV8_OUT/modules.o $DV8_SRC/modules.cc

# compile the main executable
$CC $CCFLAGS -c -o $DV8_OUT/dv8main.o $DV8_SRC/dv8_main.cc

# compile the dv8 core
$CC $CCFLAGS -c -o $DV8_OUT/dv8.o $DV8_SRC/dv8.cc

# create the lib
rm -f $DV8_OUT/dv8.a
ar crsT $DV8_OUT/dv8.a $DV8_OUT/buffer.o $DV8_OUT/env.o $DV8_OUT/dv8.o $DV8_OUT/modules.o $DV8_OUT/libz.o $DV8_OUT/loop.o $DV8_OUT/process.o $DV8_OUT/timer.o $DV8_OUT/thread.o $DV8_OUT/tty.o $DV8_OUT/os.o $DV8_OUT/fs.o $DV8_OUT/socket.o $DV8_OUT/udp.o $DV8_OUT/openssl.o
$CC -static $LDFLAGS -s -o /out/bin/dv8
rm -f $DV8_OUT/*.a
rm -f $DV8_OUT/*.o