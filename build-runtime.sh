#!/bin/bash
if test -f "build/dv8"; then
  build/dv8 -e "$(cat build.js)" docker.json
fi
# convert base js into c byte array so it can be built into binary
./builtins.sh $@
docker run -it --rm -v $(pwd)/deps:/deps -v $(pwd)/.ccache:/root/.ccache -v $(pwd)/build:/build -v $(pwd)/src:/src dv8-sdk ./platform.sh $@
