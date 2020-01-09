#!/bin/bash
CONFIG=${1:-release}
echo "building dv8 platform ($CONFIG)"
export V8_INCLUDE=/deps/v8/include
export UV_INCLUDE=/deps/uv/include
export V8_DEPS=/deps/v8
export UV_DEPS=/deps/uv
export BUILTINS=/src/builtins
export SSL_PREFIX=/usr/lib/x86_64-linux-gnu
export TRACE="TRACE=0"

if [[ "$CONFIG" == "release" ]]; then
    export CCFLAGS="-DV8_DLOPEN=0 -DHTTP_PARSER_STRICT=0 -D$TRACE -I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I/src -msse4 -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -O3 -fno-omit-frame-pointer -fno-rtti -ffast-math -fno-ident -fno-exceptions -fmerge-all-constants -fno-unroll-loops -fno-unwind-tables -fno-math-errno -fno-stack-protector -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -std=gnu++1y"
else
    export CCFLAGS="-DHTTP_PARSER_STRICT=0 -D$TRACE -I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I/src -msse4 -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -g -fno-omit-frame-pointer -fno-rtti -fno-exceptions -std=gnu++1y"
fi
export LDFLAGS="-pthread -m64 -Wl,-z,norelro -Wl,--start-group ./dv8main.o ./dv8.a $V8_DEPS/libv8_monolith.a $UV_DEPS/libuv.a $SSL_PREFIX/libssl.a $SSL_PREFIX/libcrypto.a -lz -ldl -Wl,--end-group"
export CC="ccache clang++"

# compile the builtins
$CC $CCFLAGS -c -o buffer.o /src/builtins/buffer.cc
$CC $CCFLAGS -c -o env.o /src/builtins/env.cc

# compile the core modules
$CC $CCFLAGS -I/src/modules/process -c -o process.o /src/modules/process/process.cc
$CC $CCFLAGS -I/src/modules/thread -c -o thread.o /src/modules/thread/thread.cc
$CC $CCFLAGS -I/src/modules/loop -c -o loop.o /src/modules/loop/loop.cc
$CC $CCFLAGS -I/src/modules/timer -c -o timer.o /src/modules/timer/timer.cc
$CC $CCFLAGS -I/src/modules/socket -c -o socket.o /src/modules/socket/socket.cc
$CC $CCFLAGS -I/src/modules/udp -c -o udp.o /src/modules/udp/udp.cc
$CC $CCFLAGS -I/src/modules/tty -c -o tty.o /src/modules/tty/tty.cc
$CC $CCFLAGS -I/src/modules/fs -c -o fs.o /src/modules/fs/fs.cc
$CC $CCFLAGS -I/src/modules/os -c -o os.o /src/modules/os/os.cc
$CC $CCFLAGS -I/src/modules/libz -c -o libz.o /src/modules/libz/libz.cc
$CC $CCFLAGS -I/src/modules/openssl -c -o openssl.o /src/modules/openssl/openssl.cc

# compile the main executable
$CC $CCFLAGS -c -o dv8main.o /src/dv8_main.cc

# compile the dv8 core
$CC $CCFLAGS -c -o dv8.o /src/dv8.cc

# create the lib
rm -f dv8.a
ar crsT dv8.a buffer.o env.o dv8.o libz.o loop.o process.o timer.o thread.o tty.o os.o fs.o socket.o udp.o openssl.o

# link
if [[ "$CONFIG" == "release" ]]; then
    $CC -static $LDFLAGS -s -o ./dv8-static
    $CC -rdynamic $LDFLAGS -s -o ./dv8
else
    $CC -static $LDFLAGS -o ./dv8-static
    $CC -rdynamic $LDFLAGS -o ./dv8
fi

mv ./dv8 /out/bin/dv8
mv ./dv8-static /out/bin/dv8-static
