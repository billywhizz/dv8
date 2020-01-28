#!/bin/bash
build/dv8 -e "$(cat build.js)" local.json
./platform.sh