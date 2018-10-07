# DV8

## Introduction
A low level v8 based JS runtime with small footprint (well, as small as is possible with v8)

## Build

### Build all with dependencies (v8 and libuv)
```bash
## clone the repo
git clone git@github.com:billywhizz/dv8.git
cd dv8
## build v8
docker build -t v8-build --target v8-build .
## build libuv
docker build -t uv-build --target uv-build .
## build sdk and runtime
docker build -t dv8-sdk -f Dockerfile.sdk .
## build runtime only docker image
docker build -t dv8 -f Dockerfile.runtime .
```

### Build runtime and standard libs from source
```bash
## pull the sdk image
docker pull dv8-sdk
## clone the repo
git clone git@github.com:billywhizz/dv8.git
cd dv8
## build the platform
docker run -it --rm -v $(pwd)/out/bin:/usr/local/bin -v $(pwd)/src:/src dv8-sdk ./platform.sh
## run with your build
docker run -it --rm -v $(pwd)/out/bin/dv8:/usr/local/bin/dv8 -v $(pwd)/out/lib:/usr/local/lib dv8 /bin/sh
```

### Build a module (To Be Improved)
```bash
## pull the sdk image
docker pull dv8-sdk
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
docker run -it --rm -v $(pwd):/src/module/$MODULE_NAME -v $(pwd)/out/lib:/usr/local/lib dv8-sdk ./module.sh $MODULE_NAME
```
- run with your module
```bash
docker run -it --rm -v $(pwd)/out/lib/$MODULE_NAME.so:/usr/local/lib/$MODULE_NAME.so dv8 /bin/sh
```

## Run
docker run -it --rm -v $(pwd)/app:/app dv8:latest /bin/sh

## Principles
- YAGNI
- KISS

## Summary
- docker only (for now)
- alpine only (for now)
- 15MiB binary
- 21MiB runtime docker image
- 13MiB RSS on startup
- xxx "hello world" process time
- defaults to an environment with only the core JS runtime and the following globals
    - version()
    - print()
    - module()
    - require()
    - shutdown()
    - gc()
    - Buffer
        - alloc()
        - pull()
        - push()
- the standard library is available as a set of optional shared modules
    - os
    - process
    - socket
    - timer
    - tty
    - debugger
    - fs

## Roadmap
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
