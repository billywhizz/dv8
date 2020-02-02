#!/bin/bash
CONFIG=release

export DV8_DEPS=/deps
export DV8_SRC=/src
export DV8_OUT=/build
export HTTPPARSER_INCLUDE=$DV8_DEPS/http_parser
export V8_INCLUDE=$DV8_DEPS/v8/include
export UV_INCLUDE=$DV8_DEPS/uv/include
export V8_DEPS=$DV8_DEPS/v8
export UV_DEPS=$DV8_DEPS/uv
export MINIZ_INCLUDE=$DV8_DEPS/miniz
export MBEDTLS_INCLUDE=$DV8_DEPS/mbedtls/include
export BUILTINS=$DV8_SRC/builtins
export TRACE="TRACE=0"
if [[ "$CONFIG" == "release" ]]; then
    export CCFLAGS="-D$TRACE -I$MBEDTLS_INCLUDE -I$HTTPPARSER_INCLUDE -I$MINIZ_INCLUDE -I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I$DV8_SRC -msse4 -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -O3 -fno-omit-frame-pointer -fno-rtti -ffast-math -fno-ident -fno-exceptions -fmerge-all-constants -fno-unroll-loops -fno-unwind-tables -fno-math-errno -fno-stack-protector -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -std=gnu++1y"
    export LDFLAGS="-pthread -m64 -Wl,-z,norelro -Wl,--start-group /usr/lib/x86_64-linux-gnu/libssl.a /usr/lib/x86_64-linux-gnu/libcrypto.a /deps/mbedtls/library/libmbedcrypto.a $DV8_OUT/dv8main.o $DV8_OUT/dv8.a $V8_DEPS/libv8_monolith.a $UV_DEPS/libuv.a -ldl -Wl,--end-group"
else
    export CCFLAGS="-D$TRACE -I$MBEDTLS_INCLUDE -I$HTTPPARSER_INCLUDE -I$MINIZ_INCLUDE -I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I$DV8_SRC -msse4 -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -g -fno-omit-frame-pointer -fno-rtti -fno-exceptions -std=gnu++1y"
    export LDFLAGS="-pthread -m64 -Wl,-z,norelro -Wl,--start-group /usr/lib/x86_64-linux-gnu/libssl.a /usr/lib/x86_64-linux-gnu/libcrypto.a /deps/mbedtls/library/libmbedcrypto.a $DV8_OUT/dv8main.o $DV8_OUT/dv8.a $V8_DEPS/libv8_monolith.a $UV_DEPS/libuv.a -ldl -Wl,--end-group"
fi
echo "building mbedtls"
make -C $DV8_DEPS/mbedtls/ lib
echo "building dv8 platform ($CONFIG)"
export CC="ccache g++"
$CC $CCFLAGS -c -o $DV8_OUT/buffer.o $DV8_SRC/builtins/buffer.cc
$CC $CCFLAGS -c -o $DV8_OUT/env.o $DV8_SRC/builtins/env.cc
$CC $CCFLAGS -I$DV8_SRC/modules/loop -c -o $DV8_OUT/loop.o $DV8_SRC/modules/loop/loop.cc
$CC $CCFLAGS -I$DV8_SRC/modules/timer -c -o $DV8_OUT/timer.o $DV8_SRC/modules/timer/timer.cc
$CC $CCFLAGS -I$DV8_SRC/modules/fs -c -o $DV8_OUT/fs.o $DV8_SRC/modules/fs/fs.cc
$CC $CCFLAGS -I$DV8_SRC/modules/process -c -o $DV8_OUT/process.o $DV8_SRC/modules/process/process.cc
$CC $CCFLAGS -I$DV8_SRC/modules/tty -c -o $DV8_OUT/tty.o $DV8_SRC/modules/tty/tty.cc
$CC $CCFLAGS -I$DV8_SRC/modules/libz -c -o $DV8_OUT/libz.o $DV8_SRC/modules/libz/libz.cc
$CC $CCFLAGS -I$DV8_SRC/modules/os -c -o $DV8_OUT/os.o $DV8_SRC/modules/os/os.cc
$CC $CCFLAGS -I$DV8_SRC/modules/socket -c -o $DV8_OUT/socket.o $DV8_SRC/modules/socket/socket.cc
$CC $CCFLAGS -I$DV8_SRC/modules/thread -c -o $DV8_OUT/thread.o $DV8_SRC/modules/thread/thread.cc
$CC $CCFLAGS -I$DV8_SRC/modules/udp -c -o $DV8_OUT/udp.o $DV8_SRC/modules/udp/udp.cc
$CC $CCFLAGS -I$DV8_SRC/modules/mbedtls -c -o $DV8_OUT/mbedtls.o $DV8_SRC/modules/mbedtls/mbedtls.cc
$CC $CCFLAGS -I$DV8_SRC/modules/httpParser -c -o $DV8_OUT/httpParser.o $DV8_SRC/modules/httpParser/httpParser.cc
$CC $CCFLAGS -I$DV8_SRC/modules/openssl -c -o $DV8_OUT/openssl.o $DV8_SRC/modules/openssl/openssl.cc
$CC $CCFLAGS -c -o $DV8_OUT/miniz.o $MINIZ_INCLUDE/miniz.c
$CC -DHTTP_PARSER_STRICT=0 $CCFLAGS -c -o $DV8_OUT/http_parser.o $HTTPPARSER_INCLUDE/http_parser.c
$CC $CCFLAGS -c -o $DV8_OUT/modules.o $DV8_SRC/modules.cc
$CC $CCFLAGS -c -o $DV8_OUT/dv8main.o $DV8_SRC/dv8_main.cc
$CC $CCFLAGS -c -o $DV8_OUT/dv8.o $DV8_SRC/dv8.cc
rm -f $DV8_OUT/dv8.a
ar crsT $DV8_OUT/dv8.a $DV8_OUT/buffer.o $DV8_OUT/env.o $DV8_OUT/dv8.o $DV8_OUT/modules.o $DV8_OUT/loop.o $DV8_OUT/timer.o $DV8_OUT/fs.o $DV8_OUT/process.o $DV8_OUT/tty.o $DV8_OUT/libz.o $DV8_OUT/os.o $DV8_OUT/socket.o $DV8_OUT/thread.o $DV8_OUT/udp.o $DV8_OUT/mbedtls.o $DV8_OUT/httpParser.o $DV8_OUT/openssl.o $DV8_OUT/miniz.o $DV8_OUT/http_parser.o
if [[ "$CONFIG" == "release" ]]; then
$CC -static $LDFLAGS -s -o $DV8_OUT/dv8
else
$CC -static $LDFLAGS -o $DV8_OUT/dv8
fi
