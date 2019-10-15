#!/bin/bash
CONFIG=${1:-release}
echo "building dynamic dv8 platform ($CONFIG)"
export V8_INCLUDE=/deps/v8/include
export UV_INCLUDE=/deps/uv/include
export V8_DEPS=/deps/v8
export UV_DEPS=/deps/uv
export BUILTINS=/src/builtins

if [[ "$CONFIG" == "release" ]]; then
    export CCFLAGS="-DHTTP_PARSER_STRICT=0 -I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I/src -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -O3 -fno-omit-frame-pointer -fno-rtti -fno-exceptions -std=gnu++1y"
else
    export CCFLAGS="-DHTTP_PARSER_STRICT=0 -I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I/src -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -g -fno-omit-frame-pointer -fno-rtti -fno-exceptions -std=gnu++1y"
fi
export LDFLAGS="-pthread -rdynamic -m64 -Wl,--start-group ./dv8main.o ./dv8.a $V8_DEPS/libv8_monolith.a $UV_DEPS/libuv.a -ldl -Wl,--end-group"
export CC="ccache g++"

# compile the builtins
$CC $CCFLAGS -c -o buffer.o /src/builtins/buffer.cc
$CC $CCFLAGS -c -o env.o /src/builtins/env.cc
# compile the dv8 core
$CC $CCFLAGS -c -o dv8.o /src/dv8.cc
# create the lib
rm -f dv8.a
ar crsT dv8.a buffer.o env.o dv8.o
# compile the main executable
$CC $CCFLAGS -c -o dv8main.o /src/dv8_main.cc
# link main executable
$CC $LDFLAGS -o ./dv8
if [[ "$CONFIG" == "release" ]]; then
    strip ./dv8
fi
cp -f ./dv8 /out/bin/dv8-dynamic
