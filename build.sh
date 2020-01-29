#!/bin/bash
if test -f "build/dv8"; then
  build/dv8 -e "$(cat build.js)" local.json
fi
./platform.sh