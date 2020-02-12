const { args, require } = dv8
const { readFile, writeFile, FileSystem } = require('./util.js')
const { O_CREAT, O_TRUNC, O_WRONLY, S_IRUSR, S_IWUSR, S_IXUSR } = FileSystem

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
    result = `${result}\n$C $CFLAGS -D_LARGEFILE64_SOURCE -c -o $DV8_OUT/miniz.o $MINIZ_INCLUDE/miniz.c`
  }
  if (config.modules.some(v => v === 'httpParser')) {
    result = `${result}\n$C -DHTTP_PARSER_STRICT=0 $CFLAGS -c -o $DV8_OUT/http_parser.o $HTTPPARSER_INCLUDE/http_parser.c`
  }
  if (config.modules.some(v => v === 'net')) {
    result = `${result}\n$C -msse4 -c -o $DV8_OUT/picohttpparser.o $PICOHTTP_INCLUDE/picohttpparser.c`
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
  if (config.modules.some(v => v === 'net')) {
    result = `${result} $DV8_OUT/picohttpparser.o`
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
export PICOHTTP_INCLUDE=$DV8_DEPS/picohttpparser
export V8_DEPS=$DV8_DEPS/v8
export JSYS_INCLUDE=$DV8_DEPS/jsys
export MINIZ_INCLUDE=$DV8_DEPS/miniz
export MBEDTLS_INCLUDE=$DV8_DEPS/mbedtls/include
export BUILTINS=$DV8_SRC/builtins
export TRACE="TRACE=0"
export CCFLAGS="-D$TRACE -I$PICOHTTP_INCLUDE -I$MBEDTLS_INCLUDE -I$JSYS_INCLUDE -I$HTTPPARSER_INCLUDE -I$MINIZ_INCLUDE -I$V8_INCLUDE -I$BUILTINS -I$DV8_SRC -msse4 -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 ${compilerOptions(config)}-fno-omit-frame-pointer -fno-rtti -ffast-math -fno-ident -fno-exceptions -fmerge-all-constants -fno-unroll-loops -fno-unwind-tables -fno-math-errno -fno-stack-protector -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -std=gnu++1y"
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
const buf = readFile(args[2] || 'local.json')
const config = JSON.parse(buf.read(0, buf.size))
writeFile(config.output.build || './platform.sh', Buffer.fromString(getBuildScript(config)), O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR)
writeFile(config.output.modulesHeader || './src/modules.h', Buffer.fromString(getHeader(config)))
writeFile(config.output.modulesSource || './src/modules.cc', Buffer.fromString(getSource(config)))
