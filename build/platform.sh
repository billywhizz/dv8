#!/bin/sh
echo building dv8 platform
CONFIG=${1:-release}
export V8_INCLUDE=/deps/v8/include
export UV_INCLUDE=/deps/uv/include
export V8_DEPS=/deps/v8/out.gn/x64.release/obj
export UV_DEPS=/deps/uv/out/Release
export BUILTINS=/src/builtins

if [[ "$CONFIG" == "release" ]]; then
    export CCFLAGS="-DHTTP_PARSER_STRICT=0 -DSTATIC_BUILD=1 -I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I/src -msse4 -pthread -Wall -Wextra -Wno-cast-function-type -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -O3 -fno-omit-frame-pointer -fno-rtti -fno-exceptions -std=gnu++1y"
else
    export CCFLAGS="-DHTTP_PARSER_STRICT=0 -DSTATIC_BUILD=1 -I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I/src -msse4 -pthread -Wall -Wextra -Wno-cast-function-type -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -g -fno-omit-frame-pointer -fno-rtti -fno-exceptions -std=gnu++1y"
fi
export LDFLAGS="-msse4 -pthread -static -rdynamic -m64 -Wl,--start-group ./dv8main.o ./dv8.a $V8_DEPS/libv8_monolith.a $UV_DEPS/libuv.a /usr/lib/libssl.a /usr/lib/libcrypto.a -ldl -lrt -lm -lz -Wl,--end-group"

# compile the builtins
g++ $CCFLAGS -c -o buffer.o /src/builtins/buffer.cc
g++ $CCFLAGS -c -o env.o /src/builtins/env.cc
g++ $CCFLAGS -c -o http_parser.o /src/modules/httpParser/http_parser.c

# compile the core modules
g++ $CCFLAGS -I/src/modules/process -c -o process-binding.o /src/modules/process/binding.cc
g++ $CCFLAGS -I/src/modules/process -c -o process.o /src/modules/process/process.cc

g++ $CCFLAGS -I/src/modules/thread -c -o thread-binding.o /src/modules/thread/binding.cc
g++ $CCFLAGS -I/src/modules/thread -c -o thread.o /src/modules/thread/thread.cc

g++ $CCFLAGS -I/src/modules/loop -c -o loop-binding.o /src/modules/loop/binding.cc
g++ $CCFLAGS -I/src/modules/loop -c -o loop.o /src/modules/loop/loop.cc

g++ $CCFLAGS -I/src/modules/timer -c -o timer-binding.o /src/modules/timer/binding.cc
g++ $CCFLAGS -I/src/modules/timer -c -o timer.o /src/modules/timer/timer.cc

g++ $CCFLAGS -I/src/modules/socket -c -o socket-binding.o /src/modules/socket/binding.cc
g++ $CCFLAGS -I/src/modules/socket -c -o socket.o /src/modules/socket/socket.cc

g++ $CCFLAGS -I/src/modules/udp -c -o udp-binding.o /src/modules/udp/binding.cc
g++ $CCFLAGS -I/src/modules/udp -c -o udp.o /src/modules/udp/udp.cc

g++ $CCFLAGS -I/src/modules/tty -c -o tty-binding.o /src/modules/tty/binding.cc
g++ $CCFLAGS -I/src/modules/tty -c -o tty.o /src/modules/tty/tty.cc

g++ $CCFLAGS -I/src/modules/os -c -o os-binding.o /src/modules/os/binding.cc
g++ $CCFLAGS -I/src/modules/os -c -o os.o /src/modules/os/os.cc

g++ $CCFLAGS -I/src/modules/fs -c -o fs-binding.o /src/modules/fs/binding.cc
g++ $CCFLAGS -I/src/modules/fs -c -o fs.o /src/modules/fs/fs.cc

g++ $CCFLAGS -I/src/modules/libz -c -o libz-binding.o /src/modules/libz/binding.cc
g++ $CCFLAGS -I/src/modules/libz -c -o libz.o /src/modules/libz/libz.cc

g++ $CCFLAGS -I/src/modules/socket -I/src/modules/picoHttpParser -c -o picoHttpParser-binding.o /src/modules/picoHttpParser/binding.cc
g++ $CCFLAGS -I/src/modules/socket -I/src/modules/picoHttpParser -c -o picoHttpParser.o /src/modules/picoHttpParser/picoHttpParser.cc

g++ $CCFLAGS -I/src/modules/socket -I/src/modules/httpParser -c -o httpParser-binding.o /src/modules/httpParser/binding.cc
g++ $CCFLAGS -I/src/modules/socket -I/src/modules/httpParser -c -o httpParser.o /src/modules/httpParser/httpParser.cc

g++ $CCFLAGS -I/src/modules/socket -I/src/modules/openssl -c -o openssl-binding.o /src/modules/openssl/binding.cc
g++ $CCFLAGS -I/src/modules/socket -I/src/modules/openssl -c -o openssl.o /src/modules/openssl/openssl.cc

# compile the dv8 core
g++ $CCFLAGS -c -o dv8.o /src/dv8.cc
# create the lib
rm -f dv8.a
ar crsT dv8.a buffer.o env.o dv8.o loop.o loop-binding.o process.o process-binding.o timer.o timer-binding.o thread.o thread-binding.o socket.o socket-binding.o udp.o udp-binding.o tty.o tty-binding.o os.o os-binding.o fs.o fs-binding.o libz.o libz-binding.o picoHttpParser.o picoHttpParser-binding.o httpParser.o httpParser-binding.o http_parser.o openssl-binding.o openssl.o
# compile the main executable
g++ $CCFLAGS -c -o dv8main.o /src/dv8_main.cc
# link main executable
g++ $LDFLAGS -o ./dv8
if [[ "$CONFIG" == "release" ]]; then
    strip ./dv8
fi
cp -f ./dv8 /out/bin/dv8
