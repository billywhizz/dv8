# Builtins

## Buffer

## Globals

# Modules

## httpParser
## loop
## openssl
## os
## process
## socket
## thread
## timer
## tty

# Base



# process module

## Process

DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "pid", Process::PID);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "memoryUsage", Process::MemoryUsage);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "heapUsage", Process::HeapSpaceUsage);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "cpuUsage", Process::CPUUsage);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "hrtime", Process::HRTime);

# socket module

## constants
DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, TCP), "TCP", exports);
DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, UNIX), "UNIX", exports);

## Socket class

## methods
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "listen", Listen);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "connect", Connect);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "bind", Bind);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "close", Close);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "write", Write);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", Setup);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setNoDelay", SetNoDelay);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "pause", Pause);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "resume", Resume);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setKeepAlive", SetKeepAlive);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "remoteAddress", RemoteAddress);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "error", Error);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "queueSize", QueueSize);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stats", Stats);

## events
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onConnect", onConnect);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onClose", onClose);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onDrain", onDrain);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onWrite", onWrite);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onData", onData);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onError", onError);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onEnd", onEnd);

# thread module

## Thread class

### methods
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "start", Thread::Start);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stop", Thread::Stop);

# timer module

## Timer class

### methods
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "start", Timer::Start);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stop", Timer::Stop);

# tty module
## constants
DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, 0), "UV_TTY_MODE_NORMAL", exports);
DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, 1), "UV_TTY_MODE_RAW", exports);
DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, 2), "UV_TTY_MODE_IO", exports);

## TTY class

### methods
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "write", TTY::Write);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "close", TTY::Close);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", TTY::Setup);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "pause", TTY::Pause);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "resume", TTY::Resume);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "queueSize", TTY::QueueSize);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stats", TTY::Stats);
DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "error", TTY::Error);

