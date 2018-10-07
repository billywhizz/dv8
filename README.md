# DV8

## Introduction
A low level v8 based JS runtime with small footprint (well, as small as is possible with v8)

Inspired by node.js, libuv, v8 and many other great projects built around these

Dockerfile for v8 modified from here: https://hub.docker.com/r/alexmasterov/alpine-libv8/ with some v8 build options taken from various places

The code is of a very low standard and right now this is just a proof of concept. will be adding tasks to the roadmap below.

## Run a shell
```bash
## export the version
export DV8_VERSION=0.0.4
docker run -it --rm -v $(pwd)/examples:/app billywhizz/dv8:$DV8_VERSION /bin/sh
dv8 repl.js
```

## Run TTY benchmark
```bash
docker run -it --rm -v $(pwd)/examples:/app billywhizz/dv8:$DV8_VERSION ./bench-tty.sh 1000000 65536
```
### Results - Intel(R) Core(TM) i7-6560U CPU @ 2.20GHz
- Benchmark Pipe
```bash
pipe.stdin  :  65536000000 bytes  19.896 sec   25130.67 Mib/s    3141.33 MiB/s 15.57 RSS 1.35 Heap 0.06 Ext
pipe.stdout :  65536000000 bytes  19.896 sec   25130.67 Mib/s    3141.33 MiB/s 15.57 RSS 1.35 Heap 0.06 Ext
count.stdin :  65536000000 bytes  19.896 sec   25130.67 Mib/s    3141.33 MiB/s 17.3 RSS 1.56 Heap 0.06 Ext
```
- Benchmark Count
```bash
count.stdin :  65536000000 bytes  14.326 sec   34901.57 Mib/s    4362.69 MiB/s 17.32 RSS 1.87 Heap 0.06 Ext
```

## Build

### Export the version you want to build
```bash
## export the version
export DV8_VERSION=0.0.4
```

### Build all with dependencies (v8 and libuv)
```bash
## get the source
wget https://github.com/billywhizz/dv8/archive/v$DV8_VERSION.tar.gz
tar -zxvf v$DV8_VERSION.tar.gz
cd dv8-$DV8_VERSION
## build v8
docker build -t v8-build .
## build libuv
docker build -t uv-build .
## build sdk and runtime
docker build -t dv8-sdk -f Dockerfile.sdk .
## build runtime only docker image
docker build -t dv8 -f Dockerfile.runtime .
```

### Build runtime and standard libs from source
```bash
## get the source
wget https://github.com/billywhizz/dv8/archive/v$DV8_VERSION.tar.gz
tar -zxvf v$DV8_VERSION.tar.gz
cd dv8-$DV8_VERSION
## build the platform
docker run -it --rm -v $(pwd)/out/bin:/out/bin -v $(pwd)/src:/src billywhizz/dv8-sdk:$DV8_VERSION ./platform.sh
## build the standard modules
docker run -it --rm -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src billywhizz/dv8-sdk:$DV8_VERSION ./module.sh os
docker run -it --rm -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src billywhizz/dv8-sdk:$DV8_VERSION ./module.sh process
docker run -it --rm -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src billywhizz/dv8-sdk:$DV8_VERSION ./module.sh socket
docker run -it --rm -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src billywhizz/dv8-sdk:$DV8_VERSION ./module.sh timer
docker run -it --rm -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src billywhizz/dv8-sdk:$DV8_VERSION ./module.sh tty
## run with your build
docker run -it --rm -v $(pwd)/out/bin/dv8:/usr/local/bin/dv8 -v $(pwd)/out/lib:/usr/local/lib billywhizz/dv8:$DV8_VERSION /bin/sh
```

### Build a module (To Be Improved)
```bash
## pull the sdk image
docker pull billywhizz/dv8-sdk:$DV8_VERSION
```
- create a binding.cc and main cc and h files as in standard library
- export your binding register function

```cpp
#include "os.h"

namespace dv8 {
namespace os {
  using v8::Local;
  using v8::Object;
  using v8::Value;

  void InitAll(Local<Object> exports) {
    OS::Init(exports);
  }
}
}

extern "C" {
  void* _register_os() {
    return (void*)dv8::os::InitAll;
  }
}
```
- build your module
```bash
docker run -it --rm -v $(pwd):/src/modules/$MODULE_NAME -v $(pwd)/out/lib:/out/lib billywhizz/dv8-sdk:$DV8_VERSION ./module.sh $MODULE_NAME
```
- run with your module
```bash
docker run -it --rm -v $(pwd)/out/lib/$MODULE_NAME.so:/usr/local/lib/$MODULE_NAME.so billywhizz/dv8:$DV8_VERSION /bin/sh
```

## Release
```
docker tag dv8 billywhizz/dv8:$DV8_VERSION
docker push billywhizz/dv8:$DV8_VERSION
docker tag dv8-sdk billywhizz/dv8-sdk:$DV8_VERSION
docker push billywhizz/dv8-sdk:$DV8_VERSION
```

## Principles
- YAGNI
- KISS

## Summary
- alpine docker only (for now)
- 15 MiB binary
- 21 MiB runtime docker image
- 9  MiB compressed docker image
- 13 MiB RSS on startup
- 0.01s "hello world" process time
- v8 flags supported on command line
- defaults to an environment with only the core JS runtime and the following globals
    - version() - v8 version number
    - print() - fprintf(stderr) wrapper
    - module() - import a binary module
    - require() - require a js module
    - shutdown() - shutdown the event loop
    - gc() - force garbage collection (requires --expose-gc command line argument)
    - Buffer
        - alloc() - allocate memory outside the v8 heap
        - pull() - pull a v8 string from the buffer
        - push() - push a v8 string into the buffer (should rename these to read/write)
- the standard library is available as a set of optional shared modules
    - os (poc)
    - process (poc)
    - socket (poc)
    - timer (poc)
    - tty (poc)
    - debugger (tbd)
    - fs (tbd)
    - shmem (tbd)
    - threads (tbd)
    - unicode (is this possible as a shared module?)
    - ...

## Out of the box globals
```json
[
  "Object",
  "Function",
  "Array",
  "Number",
  "parseFloat",
  "parseInt",
  "Infinity",
  "NaN",
  "undefined",
  "Boolean",
  "String",
  "Symbol",
  "Date",
  "Promise",
  "RegExp",
  "Error",
  "EvalError",
  "RangeError",
  "ReferenceError",
  "SyntaxError",
  "TypeError",
  "URIError",
  "JSON",
  "Math",
  "console",
  "ArrayBuffer",
  "Uint8Array",
  "Int8Array",
  "Uint16Array",
  "Int16Array",
  "Uint32Array",
  "Int32Array",
  "Float32Array",
  "Float64Array",
  "Uint8ClampedArray",
  "BigUint64Array",
  "BigInt64Array",
  "DataView",
  "Map",
  "Set",
  "WeakMap",
  "WeakSet",
  "Proxy",
  "Reflect",
  "decodeURI",
  "decodeURIComponent",
  "encodeURI",
  "encodeURIComponent",
  "escape",
  "unescape",
  "eval",
  "isFinite",
  "isNaN",
  "version",
  "print",
  "module",
  "require",
  "shutdown",
  "gc",
  "SharedArrayBuffer",
  "Atomics",
  "BigInt",
  "globalThis",
  "WebAssembly",
  "global",
  "Buffer"
]
```
## Roadmap/TODO
- goal is to be small and fast and as close to metal as possible in JS
- easy to build other abstractions on top of this
- core is all C++, no JS
- standard library should not cause mark/sweep gc
- return codes, not exceptions as much as possible
- top level async
- Metrics in core - exposed in standard format
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
