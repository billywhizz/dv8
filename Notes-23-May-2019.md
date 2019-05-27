## Problems
- 64 Bit Arithmetic
- FLoating Point
- blocking event loop
- streams - low overhead passing of chunks between modules
- threads - thread pool independent from libuv work queue
- file system - no good async story? even if we fix the runtime?

## Plans
- eBPF integration for easy tracing/instrumentation
- gVisor Build
- Builds for other container platforms/distros?
- running on gcr.io/distroless

## running examples
```bash
git clone git@github.com:billywhizz/dv8-examples.git
cd dv8-examples/
docker run -it --rm -v $(pwd):/app billywhizz/dv8:0.0.6 /bin/sh
# inside container
cd udp
dv8 dns.js
```