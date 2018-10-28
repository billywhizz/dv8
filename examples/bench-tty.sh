#!/bin/sh
ITER=${1:-1}
BLOCK_SIZE=${2:-1}
#V8_TRACE_OPTS="--print-bytecode --optimize-for-size --use-strict --trace-gc --max-old-space-size=8 --no-expose-wasm --always-opt --predictable --single-threaded --single-threaded-gc"
#V8_TRACE_OPTS="--optimize-for-size --use-strict --max-old-space-size=8 --no-expose-wasm --always-opt --predictable --single-threaded --single-threaded-gc"
echo Benchmark Pipe
dd if=/dev/zero count=$ITER status=none bs=$BLOCK_SIZE | dv8 $V8_TRACE_OPTS pipe.js | dv8 $V8_TRACE_OPTS count.js
echo Benchmark Count
dd if=/dev/zero count=$ITER status=none bs=$BLOCK_SIZE | dv8 $V8_TRACE_OPTS count.js