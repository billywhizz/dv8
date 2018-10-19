# Demos and Benchmarks

A selection of demos and benchmarks for the standard library

All the examples can be run by launching dv8 shell using ./run.sh

## benchmarks

### bench-tty.sh
```
./bench-tty.sh 100000 65536
```
- Uses dd to pipe N number of n size chunks into pipe.js which pipes to count.js and also just to count.js
- dd --> pipe.js --> count.js
- dd --> count.js

### bench-pipe.sh
```
./bench-pipe.sh 100000 65536
```
- Uses dd to pipe to stdin on send.js which pipes to a socket recv.js is listening on. recv.js pipes to count.js on stdout
- dd --> send.js --> {SOCKET} --> recv.js --> count.js
- This saturates 4 cores on a core i7 6560 and achieves a rate of approx 20 Gib in count.js

## tests

### verify-tty.sh
```
./verify-tty.sh
```
- creates a large (6.7 GiB) file full of random bytes
- calculates CRC32 checksum usng cksum tool
- calculates MD5 hash using md5sum tool
- calculates the same checksum and md5 hash when piping through pipe.js

### verify-socket.sh
- to be implemented

## tty demos

### count.js
```
dd if=/dev/zero count=100000 bs=65536 status=none | dv8 count.js
```
- reads all bytes from stdin and produces summary

### pipe.js
```
dd if=/dev/zero count=100000 bs=65536 status=none | dv8 pipe.js | dv8 count.js
```
- pipes all bytes on stdin to stdout and produces summary

### repl.js
```
dv8 repl.js
```
- a very rudimentary repl for running statements and inspecting variabls on the shell

## socket demos

### recv.js
```
rm -f /tmp/pipe.sock && dv8 recv.js | dv8 count.js
```
- listens on a unix domain socket and pipes the bytes to stdout. closes after receiving one connection
- currently we don't have any fs calls implemented so cannot remove the socket before listening

### send.js
```
dd if=/dev/zero count=100000 bs=65536 | dv8 send.js
```
- pipes all bytes on stdin to a unix domain socket

### proxy.js
to be implement - socket proxy

## misc demos

### signal-demo.js
- demonstrates handling events for various signals

### spawn.js
```
dv8 spawn.js thread-worker.js 2
```
- spawns n number of threads with a v8 VM running the specified script loaded in each thread
- note: by default, there are only 4 worker threads available in the uv worker pool. if you want to launch more than 2-3 threads, set the UV_THREADPOOL_SIZE env var to the number of threads you would like to have pre-allocated by uv. e.g.
```
UV_THREADPOOL_SIZE=64 dv8 spawn.js thread-worker.js 60
```

### thread-demo.js
```
dv8 thread-demo.js 2 10
```
- spawns n number of threads, passing them the duration in seconds and measures the time to script being fully loaded on each thread

### thread-worker.js
- demo thread worker script. any script can be used as a thread worker. no need for any env vars of special code

## http server demo

### http.js
```bash
# single threaded
dv8 httpd.js
# multi threaded
dv8 spawn.js httpd.js 4
```
- demo http server, using slightly modified js http parser from here: https://www.npmjs.com/package/http-parser-js
- demonstrates handling signals, handling shutdown event, graceful http shutdown
- can be run on a thread as socket library is using SO_REUSEPORT for tcp
- up to 80k zero byte responses per second on a single core
- should be a lot faster with c++ http parser (need to work out details of socket handlers first)
- to benchmark:
```
docker run -it --rm williamyeh/wrk -c 100 -t 2 -d 30 http://172.17.0.1:3000/
```

## helper libraries
### lib/base.js
- adds a number of standard functions to global
- setTimeout, setInterval, clearTimeout, clearInterval - standard JS timers
- memoryUsage - detailed v8 memory usage stats
- heapUsage - detailed v8 heap usage stats
- cpuUsage - cpu usage
- hrtime - high resolution time as BigUint64
- threadId - will always be 0 for main. is read from workerData for threads
- createBuffer - helper function to create a buffer with a single function call

### lib/http-parser.js
- this library: https://www.npmjs.com/package/http-parser-js modified to work with dv8 buffers

### lib/meter.js
- module for recording pipe (tty or socket) metrics

## TODO
- md5sum.js
- cksum.js
- verify-socket.sh