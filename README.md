# DV8

## Introduction
A low level v8 based JS runtime with small footprint (well, as small as is possible with v8)

Inspired by node.js, libuv, v8 and many other great projects built around these

Dockerfile for v8 modified from here: https://hub.docker.com/r/alexmasterov/alpine-libv8/ with some v8 build options taken from various places

The code is of a very low standard and right now this is just a proof of concept. will be adding tasks to the roadmap below.

## Quickstart Run a shell
```bash
## export the version
export DV8_VERSION=0.0.5
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
export DV8_VERSION=0.0.5
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
## build debugger (lldb)
docker build -t dv8-debug -f Dockerfile.debug .
```

## Debug a core dump
```bash
## debug build
./build-all.sh debug
## run debug container
./run.sh debug
## inside container
lldb /usr/local/bin/dv8 -c core
## inside lldb
bt
```

## example backtrace
```
* thread #1, name = 'dv8', stop reason = signal SIGILL
  * frame #0: 0x000055a363f2b1f9 dv8`v8::base::OS::Abort() at platform-posix.cc:395
    frame #1: 0x000055a363f26c49 dv8`V8_Fatal(char const*, int, char const*, ...) at logging.cc:171
    frame #2: 0x000055a3639c815a dv8`v8::Isolate::RequestGarbageCollectionForTesting(v8::Isolate::GarbageCollectionType) at api.cc:8134
    frame #3: 0x000055a3639ab4cd dv8`dv8::CollectGarbage(args=0x00007ffdefbaa7d0) at dv8.cc:202
    frame #4: 0x000055a363f5c566 dv8`::HandleApiCallHelper<false>() [inlined] v8::internal::FunctionCallbackArguments::Call(v8::internal::CallHandlerInfo*) at api-arguments-inl.h:140
    frame #5: 0x000055a363f5c434 dv8`::HandleApiCallHelper<false>() at builtins-api.cc:109
    frame #6: 0x000055a363f5d568 dv8`v8::internal::Builtin_HandleApiCall(int, v8::internal::Object**, v8::internal::Isolate*) [inlined] Builtin_Impl_HandleApiCall at builtins-api.cc:139
    frame #7: 0x000055a363f5d4d7 dv8`v8::internal::Builtin_HandleApiCall(int, v8::internal::Object**, v8::internal::Isolate*) at builtins-api.cc:127
    frame #8: 0x000055a363ebe02e dv8`Builtins_CEntry_Return1_DontSaveFPRegs_ArgvOnStack_NoBuiltinExit + 78
    frame #9: 0x00001dc57db081ae
    frame #10: 0x000055a363e2f7e3 dv8`Builtins_JSEntryTrampoline + 99
    frame #11: 0x00001dc57db059de
    frame #12: 0x000055a363b0ad79 dv8`v8::internal::Execution::Call(v8::internal::Isolate*, v8::internal::Handle<v8::internal::Object>, v8::internal::Handle<v8::internal::Object>, int, v8::internal::Handle<v8::internal::Object>*) [inlined] v8::internal::GeneratedCode<v8::internal::Object*, v8::internal::Object*, v8::internal::Object*, v8::internal::Object*, int, v8::internal::Object***>::Call(v8::internal::Object*, v8::internal::Object*, v8::internal::Object*, int, v8::internal::Object***) at simulator.h:113
    frame #13: 0x000055a363b0ad60 dv8`v8::internal::Execution::Call(v8::internal::Isolate*, v8::internal::Handle<v8::internal::Object>, v8::internal::Handle<v8::internal::Object>, int, v8::internal::Handle<v8::internal::Object>*) [inlined] Invoke at execution.cc:155
    frame #14: 0x000055a363b0ac91 dv8`v8::internal::Execution::Call(v8::internal::Isolate*, v8::internal::Handle<v8::internal::Object>, v8::internal::Handle<v8::internal::Object>, int, v8::internal::Handle<v8::internal::Object>*) [inlined] CallInternal at execution.cc:193
    frame #15: 0x000055a363b0ac89 dv8`v8::internal::Execution::Call(v8::internal::Isolate*, v8::internal::Handle<v8::internal::Object>, v8::internal::Handle<v8::internal::Object>, int, v8::internal::Handle<v8::internal::Object>*) at execution.cc:203
    frame #16: 0x000055a3639da4e6 dv8`v8::Function::Call(v8::Local<v8::Context>, v8::Local<v8::Value>, int, v8::Local<v8::Value>*) at api.cc:5018
    frame #17: 0x000055a3639da6a1 dv8`v8::Function::Call(v8::Local<v8::Value>, int, v8::Local<v8::Value>*) at api.cc:5028
    frame #18: 0x00007fd3dac871a1 timer.so`dv8::timer::Timer::OnTimeout(handle=0x000055a365809900) at timer.cc:116
    frame #19: 0x000055a36429a575 dv8`uv__run_timers(loop=0x000055a3647008e0) at timer.c:174
    frame #20: 0x000055a36428ad04 dv8`uv_run(loop=0x000055a3647008e0, mode=UV_RUN_DEFAULT) at core.c:361
    frame #21: 0x000055a3639a5d76 dv8`main(argc=2, argv=0x00007ffdefbab1c8) at dv8_main.cc:63
    frame #22: 0x00007fd3db0b0ad6 ld-musl-x86_64.so.1`__libc_start_main + 54
    frame #23: 0x000055a3639a5102 dv8`_start_c(p=<unavailable>) at crt1.c:17
    frame #24: 0x000055a3639a50da dv8`_start + 22
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
        - alloc(size) - allocate memory outside the v8 heap
        - read(len) - read a v8 string from the buffer
        - write(str) - write a v8 string into the buffer
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
# API

## JS

### main.global
  version()
  print()
  module()
  require()
  shutdown()
  gc()
### thread.global
  version()
  print()
  module()
  require()
### os
  onSignal()
### process
  pid()
  memoryUsage()
### socket
  listen()
  connect()
  bind()
  close()
  pull()
  push()
  write()
  writeText()
  setup()
  setNoDelay()
  pause()
  resume()
  setKeepAlive()
  proxy()
  remoteAddress()
  onConnect()
  onClose()
  onWrite()
  onData()
  onError()
### thread
  start()
### timer
  start()
  stop()
### tty
  writeString()
  write()
  close()
  setup()
  pause()
  resume()
  queueSize()
  stats()
  UV_TTY_MODE_NORMAL
  UV_TTY_MODE_RAW
  UV_TTY_MODE_IO


## C++

### global
dv8::Version()
dv8::Require()

### builtins

### modules


# Tests

## netcat/pipecat

```bash
# run pipecat
rm -f /tmp/pipe.sock & dv8 pipecat.js | cat 1> /dev/null
# test pipecat using socat
dd if=/dev/zero count=100000 bs=65536 | socat - unix:/tmp/pipe.sock
# test pipecat using netcat
dd if=/dev/zero count=100000 bs=65536 | dv8 netcat.js
# test netcat using socat
rm -f /tmp/pipe.sock & socat unix-listen:/tmp/pipe.sock - | cat 1> /dev/null

```