#!/bin/bash
CONFIG=${1:-release}
echo "building dv8 platform ($CONFIG)"
export V8_INCLUDE=/deps/v8/include
export UV_INCLUDE=/deps/uv/include
export V8_DEPS=/deps/v8
export UV_DEPS=/deps/uv
export BUILTINS=/src/builtins

if [[ "$CONFIG" == "release" ]]; then
    export CCFLAGS="-I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I/src -msse4 -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -O3 -fno-omit-frame-pointer -fno-rtti -fno-exceptions -std=gnu++1y"
else
    export CCFLAGS="-I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I/src -msse4 -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -g -fno-omit-frame-pointer -fno-rtti -fno-exceptions -std=gnu++1y"
fi
export LDFLAGS="-pthread -static -m64 -Wl,--start-group ./dv8main.o ./dv8.a $V8_DEPS/libv8_monolith.a $UV_DEPS/libuv.a -ldl -Wl,--end-group"
export LDFLAGS_DYNAMIC="-pthread -rdynamic -m64 -Wl,--start-group ./dv8main.o ./dv8.a $V8_DEPS/libv8_monolith.a $UV_DEPS/libuv.a -ldl -Wl,--end-group"
export CC="ccache g++"

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
$CC $CCFLAGS -I/src/modules/os -c -o os.o /src/modules/os/os.cc
$CC $CCFLAGS -I/src/modules/fs -c -o fs.o /src/modules/fs/fs.cc

# compile the main executable
$CC $CCFLAGS -c -o dv8main.o /src/dv8_main.cc

# compile the dv8 core
$CC $CCFLAGS -c -o dv8.o /src/dv8.cc

# create the lib
rm -f dv8.a

ar crsT dv8.a buffer.o env.o dv8.o loop.o process.o timer.o thread.o tty.o os.o fs.o socket.o udp.o

# link static binaru
$CC $LDFLAGS -o ./dv8

# compile the dv8 core
$CC $CCFLAGS -DDLOPEN=1 -c -o dv8.o /src/dv8.cc

# create the lib
rm -f dv8.a

ar crsT dv8.a buffer.o env.o dv8.o loop.o process.o timer.o thread.o tty.o os.o fs.o socket.o udp.o

# link dynamic binaru
$CC $LDFLAGS_DYNAMIC -o ./dv8-dynamic

# strip symbols in release binary
if [[ "$CONFIG" == "release" ]]; then
    strip ./dv8
    strip ./dv8-dynamic
fi
mv ./dv8-dynamic /out/bin/dv8-dynamic
mv ./dv8 /out/bin/dv8
