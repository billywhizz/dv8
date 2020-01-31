#!/bin/bash
if test -f "build/dv8"; then
  echo building config
  build/dv8 -e "$(cat build.js)" local.json
fi
./platform.sh