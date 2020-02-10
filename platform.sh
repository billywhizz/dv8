#!/bin/bash
./builtins.sh 
export DV8_DEPS=./deps
export DV8_SRC=./src
export DV8_OUT=./bin
export HTTPPARSER_INCLUDE=$DV8_DEPS/http_parser
export V8_INCLUDE=$DV8_DEPS/v8/include
export PICOHTTP_INCLUDE=$DV8_DEPS/picohttpparser
export V8_DEPS=$DV8_DEPS/v8
export JSYS_INCLUDE=$DV8_DEPS/jsys
export MINIZ_INCLUDE=$DV8_DEPS/miniz
export MBEDTLS_INCLUDE=$DV8_DEPS/mbedtls/include
export BUILTINS=$DV8_SRC/builtins
export TRACE="TRACE=0"
export CCFLAGS="-D$TRACE -I$PICOHTTP_INCLUDE -I$MBEDTLS_INCLUDE -I$JSYS_INCLUDE -I$HTTPPARSER_INCLUDE -I$MINIZ_INCLUDE -I$V8_INCLUDE -I$BUILTINS -I$DV8_SRC -msse4 -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -g -fno-omit-frame-pointer -fno-rtti -ffast-math -fno-ident -fno-exceptions -fmerge-all-constants -fno-unroll-loops -fno-unwind-tables -fno-math-errno -fno-stack-protector -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -std=gnu++1y"
export LDFLAGS="-pthread -m64 -Wl,-z,norelro -Wl,--start-group  $DV8_OUT/dv8main.o $DV8_OUT/dv8.a $V8_DEPS/libv8_monolith.a -Wl,--end-group"
echo "building mbedtls"
make -C $DV8_DEPS/mbedtls/ lib
echo "building dv8 platform (release)"
export CC="ccache g++"
export C="ccache gcc"
export CFLAGS="-Wall -Wextra"
$CC $CCFLAGS -c -o $DV8_OUT/buffer.o $DV8_SRC/builtins/buffer.cc
$CC $CCFLAGS -c -o $DV8_OUT/env.o $DV8_SRC/builtins/env.cc
$CC $CCFLAGS -I$DV8_SRC/modules/tty -c -o $DV8_OUT/tty.o $DV8_SRC/modules/tty/tty.cc
$CC $CCFLAGS -I$DV8_SRC/modules/loop -c -o $DV8_OUT/loop.o $DV8_SRC/modules/loop/loop.cc
$CC $CCFLAGS -I$DV8_SRC/modules/timer -c -o $DV8_OUT/timer.o $DV8_SRC/modules/timer/timer.cc
$CC $CCFLAGS -I$DV8_SRC/modules/fs -c -o $DV8_OUT/fs.o $DV8_SRC/modules/fs/fs.cc
$CC $CCFLAGS -I$DV8_SRC/modules/net -c -o $DV8_OUT/net.o $DV8_SRC/modules/net/net.cc
$CC $CCFLAGS -I$DV8_SRC/modules/thread -c -o $DV8_OUT/thread.o $DV8_SRC/modules/thread/thread.cc
$C -msse4 -c -o $DV8_OUT/picohttpparser.o $PICOHTTP_INCLUDE/picohttpparser.c
$CC $CCFLAGS -c -o $DV8_OUT/modules.o $DV8_SRC/modules.cc
$CC $CCFLAGS -c -o $DV8_OUT/dv8main.o $DV8_SRC/dv8_main.cc
$CC $CCFLAGS -c -o $DV8_OUT/dv8.o $DV8_SRC/dv8.cc
rm -f $DV8_OUT/dv8.a
ar crsT $DV8_OUT/dv8.a $DV8_OUT/buffer.o $DV8_OUT/env.o $DV8_OUT/dv8.o $DV8_OUT/modules.o $DV8_OUT/tty.o $DV8_OUT/loop.o $DV8_OUT/timer.o $DV8_OUT/fs.o $DV8_OUT/net.o $DV8_OUT/thread.o $DV8_OUT/picohttpparser.o
$CC $LDFLAGS -o $DV8_OUT/dv8
