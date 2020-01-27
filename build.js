const { library } = dv8
const { File, FileSystem } = library('fs', {})
const { O_CREAT, O_TRUNC, O_RDONLY, O_WRONLY, S_IRUSR, S_IWUSR, S_IXUSR } = FileSystem

function alloc (size, shared) {
  const buf = Buffer.create()
  buf.bytes = shared ? buf.allocShared(size) : buf.alloc(size)
  buf.size = buf.size()
  return buf
}

function fromString (str, shared) {
  const buf = Buffer.alloc(str.length, shared)
  buf.write(str)
  return buf
}

Buffer.alloc = alloc
Buffer.fromString = fromString

function readFile (path) {
  const file = new File()
  const BUFSIZE = 4096
  const buf = Buffer.alloc(BUFSIZE)
  const parts = []
  file.setup(buf, buf)
  file.fd = file.open(path, O_RDONLY)
  if (file.fd < 0) throw new Error(`Error opening ${path}: ${file.fd}`)
  file.size = 0
  let len = file.read(BUFSIZE, file.size)
  while (len > 0) {
    file.size += len
    parts.push(buf.read(0, len))
    len = file.read(BUFSIZE, file.size)
  }
  if (len < 0) throw new Error(`Error reading ${path}: ${len}`)
  file.close()
  return parts.join('')
}

function getModuleCompiles (config) {
  return config.modules.map(name => `$CC $CCFLAGS -I$DV8_SRC/modules/${name} -c -o $DV8_OUT/${name}.o $DV8_SRC/modules/${name}/${name}.cc`).join('\n')
}

function getLinkLine (config) {
  const libs = config.modules.map(name => `$DV8_OUT/${name}.o`).join(' ')
  return `ar crsT $DV8_OUT/dv8.a $DV8_OUT/buffer.o $DV8_OUT/env.o $DV8_OUT/dv8.o $DV8_OUT/modules.o ${libs}`
}

function getBuiltins (config) {
  let args = `${config.target}`
  if (config.override) {
    args = `${args} override`
    if (config.override.path) {
      args = `${args} ${config.override.path}`
    }
  }
  return args
}

function getInitLibrary (config) {
  return config.modules.map(name => `if (strcmp("${name}", module_name) == 0) {\ndv8::${name}::InitAll(exports);\nreturn;\n}`).join('\n')
}

function getIncludes (config) {
  return config.modules.map(name => `#include <modules/${name}/${name}.h>`).join('\n')
}

function getBuildScript (config) {
  return `#!/bin/bash
CONFIG=${config.target}
echo "generating main.js"
./builtins.sh ${getBuiltins(config)}
echo "building dv8 platform ($CONFIG)"
export DV8_DEPS=${config.deps}
export DV8_SRC=${config.src}
export DV8_OUT=${config.build}
export V8_INCLUDE=$DV8_DEPS/v8/include
export UV_INCLUDE=$DV8_DEPS/uv/include
export V8_DEPS=$DV8_DEPS/v8
export UV_DEPS=$DV8_DEPS/uv
export BUILTINS=$DV8_SRC/builtins
export SSL_PREFIX=${config.sslPrefix || '/usr/lib/x86_64-linux-gnu'}
export TRACE="TRACE=0"
export STATIC="DV8STATIC=1"
if [[ "$CONFIG" == "release" ]]; then
    export CCFLAGS="-D$STATIC -D$TRACE -I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I$DV8_SRC -msse4 -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -O3 -fno-omit-frame-pointer -fno-rtti -ffast-math -fno-ident -fno-exceptions -fmerge-all-constants -fno-unroll-loops -fno-unwind-tables -fno-math-errno -fno-stack-protector -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -std=gnu++1y"
    export LDFLAGS="-pthread -m64 -Wl,-z,norelro -Wl,--start-group $SSL_PREFIX/libssl.a $SSL_PREFIX/libcrypto.a $DV8_OUT/dv8main.o $DV8_OUT/dv8.a $V8_DEPS/libv8_monolith.a $UV_DEPS/libuv.a -lz -ldl -Wl,--end-group"
else
    export CCFLAGS="-D$STATIC -D$TRACE -I$V8_INCLUDE -I$UV_INCLUDE -I$BUILTINS -I$DV8_SRC -msse4 -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -g -fno-omit-frame-pointer -fno-rtti -fno-exceptions -std=gnu++1y"
    export LDFLAGS="-pthread -m64 -Wl,-z,norelro -Wl,--start-group $SSL_PREFIX/libssl.a $SSL_PREFIX/libcrypto.a $DV8_OUT/dv8main.o $DV8_OUT/dv8.a $V8_DEPS/libv8_monolith.a $UV_DEPS/libuv.a -lz -ldl -Wl,--end-group"
fi
export CC="${config.CC}"
$CC $CCFLAGS -c -o $DV8_OUT/buffer.o $DV8_SRC/builtins/buffer.cc
$CC $CCFLAGS -c -o $DV8_OUT/env.o $DV8_SRC/builtins/env.cc
${getModuleCompiles(config)}
$CC $CCFLAGS -c -o $DV8_OUT/modules.o $DV8_SRC/modules.cc
$CC $CCFLAGS -c -o $DV8_OUT/dv8main.o $DV8_SRC/dv8_main.cc
$CC $CCFLAGS -c -o $DV8_OUT/dv8.o $DV8_SRC/dv8.cc
rm -f $DV8_OUT/dv8.a
${getLinkLine(config)}
$CC -static $LDFLAGS -s -o out/bin/dv8
rm -f $DV8_OUT/*.a
rm -f $DV8_OUT/*.o`
}

function getHeader (config) {
  return `#ifndef DV8_MODULES_H
#define DV8_MODULES_H

#include <v8.h>
#include <uv.h>
${getIncludes(config)}

namespace dv8 {

using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::HandleScope;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Local;
using v8::Exception;

typedef void *(*register_plugin)();
using InitializerCallback = void (*)(Local<Object> exports);

void LoadModule(const FunctionCallbackInfo<Value> &args);

}
#endif
`
}

function getSource (config) {
  return `#include <modules.h>

namespace dv8 {

inline void initLibrary(Local<Object> exports, const char* module_name) {
  ${getInitLibrary(config)}
}

void LoadModule(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  String::Utf8Value str(args.GetIsolate(), args[0]);
  const char *module_name = *str;
  Local<Object> exports;
  bool ok = args[1]->ToObject(context).ToLocal(&exports);
  if (!ok) {
    args.GetReturnValue().Set(Null(isolate));
    return;
  }
  args.GetReturnValue().Set(exports);
#if DV8STATIC
  initLibrary(exports, module_name);
#else
  const char *module_path = "/usr/local/lib/dv8/";
  char lib_name[1024];
  uv_lib_t lib;
  int success = 0;
  args.GetReturnValue().Set(exports);
  if (args.Length() > 2) {
    String::Utf8Value str(args.GetIsolate(), args[2]);
    module_path = *str;
    snprintf(lib_name, 1024, "%s%s.so", module_path, module_name);
    success = uv_dlopen(lib_name, &lib);
  } else {
    initLibrary(exports, module_name);
    snprintf(lib_name, 1024, "%s%s.so", module_path, module_name);
    success = uv_dlopen(NULL, &lib);
  }
  if (success != 0) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "uv_dlopen failed").ToLocalChecked()));
    return;
  }
  char register_name[128];
  snprintf(register_name, 128, "_register_%s", module_name);
  void *address;
  success = uv_dlsym(&lib, register_name, &address);
  if (success != 0) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "uv_dlsym failed").ToLocalChecked()));
    return;
  }
  register_plugin _init = reinterpret_cast<register_plugin>(address);
  auto _register = reinterpret_cast<InitializerCallback>(_init());
  _register(exports);
#endif
}

}
`
}

function writeFileSync (fileName, buf, flags = O_CREAT | O_TRUNC | O_WRONLY, mode = S_IRUSR | S_IWUSR) {
  const file = new File()
  file.setup(buf, buf)
  file.fd = file.open(fileName, flags, mode)
  file.write(buf.size, 0)
  file.close()
}

const config = JSON.parse(readFile('./config.json'))
writeFileSync('./platform.sh', Buffer.fromString(getBuildScript(config)), O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR)
writeFileSync('./src/modules.h', Buffer.fromString(getHeader(config)))
writeFileSync('./src/modules.cc', Buffer.fromString(getSource(config)))
