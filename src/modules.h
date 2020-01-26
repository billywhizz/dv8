#ifndef DV8_MODULES_H
#define DV8_MODULES_H

#include <v8.h>
#include <uv.h>

#include <modules/loop/loop.h>
#include <modules/timer/timer.h>
#include <modules/thread/thread.h>
#include <modules/process/process.h>
#include <modules/tty/tty.h>
#include <modules/os/os.h>
#include <modules/fs/fs.h>
#include <modules/socket/socket.h>
#include <modules/udp/udp.h>
#include <modules/libz/libz.h>
#include <modules/openssl/openssl.h>

namespace dv8 {

using v8::ArrayBuffer;
using v8::Context;
using v8::Float64Array;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::HeapStatistics;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Maybe;
using v8::MaybeLocal;
using v8::Message;
using v8::Module;
using v8::NewStringType;
using v8::Object;
using v8::ObjectTemplate;
using v8::Script;
using v8::ScriptCompiler;
using v8::ScriptOrigin;
using v8::String;
using v8::TryCatch;
using v8::V8;
using v8::Value;
using v8::PromiseRejectMessage;
using v8::PromiseRejectEvent;
using v8::HeapSpaceStatistics;
using v8::Exception;
using v8::Promise;
using v8::Global;
using v8::Array;
using v8::BigUint64Array;
using v8::Platform;

typedef void *(*register_plugin)();
using InitializerCallback = void (*)(Local<Object> exports);

void LoadModule(const FunctionCallbackInfo<Value> &args);

}
#endif
