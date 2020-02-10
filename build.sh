#!/bin/bash
if test -f "bin/dv8"; then
  echo building config
  bin/dv8 tools/build.js tools/local.json
fi
./platform.sh