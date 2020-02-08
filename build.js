const { args, require } = dv8
const { readFile, writeFile, FileSystem } = require('./util.js')
const { O_CREAT, O_TRUNC, O_WRONLY, S_IRUSR, S_IWUSR, S_IXUSR } = FileSystem

/*

  const char *module_path = "/usr/local/lib/dv8/";
  char lib_name[1024];
  if (args.Length() > 2) {
    String::Utf8Value str(args.GetIsolate(), args[2]);
    module_path = *str;
    snprintf(lib_name, 1024, "%s%s.so", module_path, module_name);
  } else {
    if (strcmp("loop", module_name) == 0) {
      dv8::loop::InitAll(exports);
      return;
    } else if (strcmp("socket", module_name) == 0) {
      dv8::socket::InitAll(exports);
      return;
    } else if (strcmp("timer", module_name) == 0) {
      dv8::timer::InitAll(exports);
      return;
    } else if (strcmp("zlib", module_name) == 0) {
      dv8::libz::InitAll(exports);
      return;
    } else if (strcmp("thread", module_name) == 0) {
      dv8::thread::InitAll(exports);
      return;
    } else if (strcmp("fs", module_name) == 0) {
      dv8::fs::InitAll(exports);
      return;
    } else if (strcmp("udp", module_name) == 0) {
      dv8::udp::InitAll(exports);
      return;
    } else if (strcmp("process", module_name) == 0) {
      dv8::process::InitAll(exports);
      return;
    } else if (strcmp("tty", module_name) == 0) {
      dv8::tty::InitAll(exports);
      return;
    } else if (strcmp("openssl", module_name) == 0) {
      dv8::openssl::InitAll(exports);
      return;
    } else if (strcmp("os", module_name) == 0) {
      dv8::os::InitAll(exports);
      return;
    }
    snprintf(lib_name, 1024, "%s%s.so", module_path, module_name);
  }
  uv_lib_t lib;
  args.GetReturnValue().Set(exports);
  fprintf(stderr, "%s\n", lib_name);
  int success = uv_dlopen(lib_name, &lib);
  if (success != 0) {
    isolate->ThrowException(v8::Exception::Error(String::NewFromUtf8(isolate, "uv_dlopen failed").ToLocalChecked()));
    return;
  }
  char register_name[128];
  snprintf(register_name, 128, "_register_%s", module_name);
  void *address;
  success = uv_dlsym(&lib, register_name, &address);
  if (success != 0) {
    isolate->ThrowException(v8::Exception::Error(String::NewFromUtf8(isolate, "uv_dlsym failed").ToLocalChecked()));
    return;
  }
  register_plugin _init = reinterpret_cast<register_plugin>(address);
  auto _register = reinterpret_cast<InitializerCallback>(_init());
  _register(exports);

*/

function alloc (size, shared) {
  const buf = Buffer.create()
  buf.bytes = shared ? buf.allocShared(size) : buf.alloc(size)
  buf.size = buf.size()
  return buf
}

function fromString (str, shared) {
  const buf = Buffer.alloc(str.length, shared)
  buf.write(str, 0)
  return buf
}

Buffer.alloc = alloc
Buffer.fromString = fromString

function getModuleCompiles (config) {
  let result = config.modules.map(name => `$CC $CCFLAGS -I$DV8_SRC/modules/${name} -c -o $DV8_OUT/${name}.o $DV8_SRC/modules/${name}/${name}.cc`).join('\n')
  if (config.modules.some(v => v === 'libz')) {
    result = `${result}\n$C $CFLAGS -c -o $DV8_OUT/miniz.o $MINIZ_INCLUDE/miniz.c`
  }
  if (config.modules.some(v => v === 'httpParser')) {
    result = `${result}\n$C -DHTTP_PARSER_STRICT=0 $CFLAGS -c -o $DV8_OUT/http_parser.o $HTTPPARSER_INCLUDE/http_parser.c`
  }
  return result
}

function getLinkLine (config) {
  const libs = config.modules.map(name => `$DV8_OUT/${name}.o`).join(' ')
  let result = `ar crsT $DV8_OUT/dv8.a $DV8_OUT/buffer.o $DV8_OUT/env.o $DV8_OUT/dv8.o $DV8_OUT/modules.o ${libs}`
  if (config.modules.some(v => v === 'libz')) {
    result = `${result} $DV8_OUT/miniz.o`
  }
  if (config.modules.some(v => v === 'httpParser')) {
    result = `${result} $DV8_OUT/http_parser.o`
  }
  return result
}

function getBuiltins (config) {
  if (!config.builtins) return ''
  let args = ''
  if (config.override) {
    args = `${args} override`
    if (config.override.path) {
      args = `${args} ${config.override.path}`
    }
  }
  return `./builtins.sh ${args}`
}

function getInitLibrary (config) {
  return config.modules.map(name => `if (strcmp("${name}", module_name) == 0) {\ndv8::${name}::InitAll(exports);\nreturn;\n}`).join('\n')
}

function getIncludes (config) {
  return config.modules.map(name => `#include <modules/${name}/${name}.h>`).join('\n')
}

function getLibs (config) {
  return config.libs.join(' ')
}

function compilerOptions (config) {
  if (config.compilerOptions) return `${config.compilerOptions} `
  return ''
}

function linkOptions (config) {
  if (config.linkOptions) return `${config.linkOptions} `
  return ''
}

function getBuildScript (config) {
  return `#!/bin/bash
${getBuiltins(config)}
export DV8_DEPS=${config.deps}
export DV8_SRC=${config.src}
export DV8_OUT=${config.build}
export HTTPPARSER_INCLUDE=$DV8_DEPS/http_parser
export V8_INCLUDE=$DV8_DEPS/v8/include
export V8_DEPS=$DV8_DEPS/v8
export JSYS_INCLUDE=../jsys/include
export MINIZ_INCLUDE=$DV8_DEPS/miniz
export JSYS_IN
export MBEDTLS_INCLUDE=$DV8_DEPS/mbedtls/include
export BUILTINS=$DV8_SRC/builtins
export TRACE="TRACE=0"
export CCFLAGS="-D$TRACE -I$MBEDTLS_INCLUDE -I$JSYS_INCLUDE -I$HTTPPARSER_INCLUDE -I$MINIZ_INCLUDE -I$V8_INCLUDE -I$BUILTINS -I$DV8_SRC -msse4 -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 ${compilerOptions(config)}-fno-omit-frame-pointer -fno-rtti -ffast-math -fno-ident -fno-exceptions -fmerge-all-constants -fno-unroll-loops -fno-unwind-tables -fno-math-errno -fno-stack-protector -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -std=gnu++1y"
export LDFLAGS="-pthread -m64 -Wl,-z,norelro -Wl,--start-group ${getLibs(config)} $DV8_OUT/dv8main.o $DV8_OUT/dv8.a $V8_DEPS/libv8_monolith.a -Wl,--end-group"
echo "building mbedtls"
make -C $DV8_DEPS/mbedtls/ lib
echo "building dv8 platform (${config.target})"
export CC="${config.CC || 'g++'}"
export C="${config.C || 'gcc'}"
export CFLAGS="-Wall -Wextra"
$CC $CCFLAGS -c -o $DV8_OUT/buffer.o $DV8_SRC/builtins/buffer.cc
$CC $CCFLAGS -c -o $DV8_OUT/env.o $DV8_SRC/builtins/env.cc
${getModuleCompiles(config)}
$CC $CCFLAGS -c -o $DV8_OUT/modules.o $DV8_SRC/modules.cc
$CC $CCFLAGS -c -o $DV8_OUT/dv8main.o $DV8_SRC/dv8_main.cc
$CC $CCFLAGS -c -o $DV8_OUT/dv8.o $DV8_SRC/dv8.cc
rm -f $DV8_OUT/dv8.a
${getLinkLine(config)}
$CC $LDFLAGS ${linkOptions(config)}-o $DV8_OUT/${config.out || 'dv8'}
`
}

function getHeader (config) {
  return `#ifndef DV8_MODULES_H
#define DV8_MODULES_H

#include <v8.h>
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
  v8::Isolate *isolate = args.GetIsolate();
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
  initLibrary(exports, module_name);
}

}
`
}

const buf = readFile((args.length > 3 ? args[3] : args[1]) || './docker.json')
const config = JSON.parse(buf.read(0, buf.size))
writeFile(config.output.build || './platform.sh', Buffer.fromString(getBuildScript(config)), O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR)
writeFile(config.output.modulesHeader || './src/modules.h', Buffer.fromString(getHeader(config)))
writeFile(config.output.modulesSource || './src/modules.cc', Buffer.fromString(getSource(config)))
