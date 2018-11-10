## rules of dv8
- docker only
- alpine docker only
- nothing in core binary but essentials
- everything else is a module
- binary goes in /usr/local/bin
- modules go in /usr/local/lib
- imports can be anywhere on local filesystem
- standard libraries as modules in monorepo
- standard libraries are shared objects dynamically loaded
- no module caching
- no cross platform - initially at least
- smallest possible code base - easy to grok
- use c over c++ where possible
- no complex abstractions
- no exceptions unless necessary
- core and standard libraries will not allocate on the heap 
- openssl supports absolute minimal set of secure protocols and ciphers
- uses platform openssl
- uses platform stdlib


## NEXT

- http parser with tls - plugin model
- tls client
- optimise/refactor everything
- update examples
- comment code
- write docs
- release 0.5

use dv8 itself to create the build scripts and run the build (shell to docker) for modules, based on a json config

all you need is:
docker
docker pull dv8
docker pull dv8-sdk

- pass args to thread function
    - receive result/messages from thread function
    - serdes
- refactor - read v8/libuv docs. handle all errors
- thread signals
- thread messaging
- replaceable bootstrap js
- fix the repl eval scope
- documentation
    - JS API
    - C++ API
    - Overview
    - Deep Dive
        - Runtime
        - Builtins
        - Threads
        - Sockets
        - Pipes
        - Timers
- figure out freeing buffer memory
- fork/exec
- thread.global functions for gc/shutdown
- segfault stack trace: https://github.com/ddopson/node-segfault-handler/blob/master/src/segfault-handler.cpp
- ssl/tls for sockets
- websockets
- inspector PoC

## v8 source
v8 source

isolates

https://v8docs.nodesource.com/node-10.6/d4/da0/v8_8h_source.html#l06874

resource constraints
https://v8docs.nodesource.com/node-10.6/d8/dcd/classv8_1_1_resource_constraints.html
A set of constraints that specifies the limits of the runtime's memory use. You must set the heap size before initializing the VM - the size cannot be adjusted after the VM is initialized.
If you are using threads then you should hold the V8::Locker lock while setting the stack limit and you must set a non-default stack limit separately for each thread.
The arguments for set_max_semi_space_size, set_max_old_space_size, set_max_executable_size, set_code_range_size specify limits in MB.
The argument for set_max_semi_space_size_in_kb is in KB.

arraybuffer allocator
https://v8docs.nodesource.com/node-10.6/d4/da0/v8_8h_source.html#l04305

## clustering
one process per core
N threads/contexts per process
threads will be scheduled by OS and will not starve each other
can we limit each thread?
if a certain thread is becoming a noisy neighbour we can use set_thread_affinity to move it to a different core
send a signal/message to the process: { threadId: 0, affinityMask: 0x00000001 }
    the process will change the thread affinity for that thread
send a message to the process to spawn a context: { home: '/foo', version: '1.0.0', main: 'app.js', port: 0 }
    process spawns a thread
        thread listens on a port (if assigned port is zero)
            thread informs process of port number it is listening on
                process sends message to controller/agent to let it know details of new thread: { pid: 20, threadId: 100, home, version, main, port: 34403 }

service registry
{ host, }

instance registry



## Roadmap/TODO
- goal is to be small and fast and as close to metal as possible in JS
- easy to build other abstractions on top of this
- core is all C++, no JS
  - maybe an optional js bootstrap that can be compiled in at build time
- no event emitters
- no streams
- no big abtractions - only the base api's in core
- standard library should not cause mark/sweep gc
- return codes, not exceptions as much as possible
- module interop - in c++ - how?
  - modules need to be able to inherit from each other
  - need to be able to wrap a module (e.g. socket) and implement on top of it (e.g. tls)
- Metrics in core - exposed in standard format
  - recorded in buffers in c++ land
  - JS land can read the metrics whenever it needs to
  - DataViews?
- secure, small, fast
- iOT support - arm build
- static modules for packaged binary
- require only supports local filesystem
- module only support local filesystem
- AtExit support
- standardize exception handling
- better hooks for modules
- expose event loop to js
- expose as much of libuv as possible to js
- die on uncaught errors
- sane threading support
- hot reloading of contexts - maintain connections/handles. investigate
- benchmarks
- releases in sync with v8?
- sane mechanism for ref'ing handles
- minimal abstractions - no common streams library - make api's for each
- figure out how to do the sockets module better - Socket instance for each connection, no fd's

## Misc
testing pipecat/netcat/count

rm -f /tmp/pipe.sock
dv8 pipecat.js | dv8 count.js
dd if=/dev/zero count=300000 bs=65536 | dv8 netcat.js


netcat test

assert(stdout.written === stdin.read)
assert(stdout.incomplete + stdout.full + stdout.eagain === stdin.data)
assert(stdin.pause === stdin.resume - 1)
assert(stdin.close === 1)
assert(stdin.error === 0)
assert(stdin.end === 1)
assert(stdout.close === 1)
assert(stdout.error === 0)
assert(stdout.alloc === stdout.free)


pipecat test

stdin.read === netcat.stdout.written


being able to make a prediction about what will happen in a given
scenario is probably the most import aspect of programming. if the code
is too complex it's very hard to make these predictions and to verify them

tests

dd if=/dev/urandom of=test.bin count=10000 bs=65536
cat test.bin | dv8 count.js
verify bytes
md5 check
crc check

counts the bytes in a 6.5 GB file in 2.5 seconds - 20Gb/sec
