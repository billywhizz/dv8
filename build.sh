#!/bin/bash
if test -f "bin/dv8"; then
  echo building config
  bin/dv8 -e "$(cat build.js)" local.json
fi
./platform.sh