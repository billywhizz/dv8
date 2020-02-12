#ifndef DV8_MODULES_H
#define DV8_MODULES_H

#include <v8.h>
#include <modules/fs/fs.h>
#include <modules/libz/libz.h>
#include <modules/loop/loop.h>
#include <modules/crypto/crypto.h>
#include <modules/memory/memory.h>
#include <modules/net/net.h>
#include <modules/os/os.h>
#include <modules/thread/thread.h>
#include <modules/timer/timer.h>
#include <modules/tty/tty.h>
#include <modules/udp/udp.h>

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
