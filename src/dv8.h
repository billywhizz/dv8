#ifndef DV8_H
#define DV8_H

#include <assert.h>
#include <unistd.h>
#include <v8.h>
#include <libplatform/libplatform.h>
#include <uv.h>
#include <common.h>
#include <buffer.h>
#include <env.h>
#include <string.h>

#define MICROS_PER_SEC 1e6

extern char **environ;
namespace dv8
{

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
using dv8::builtins::Environment;
using v8::HeapSpaceStatistics;
using v8::Exception;
using v8::Promise;

using InitializerCallback = void (*)(Local<Object> exports);


typedef struct
{
  uv_write_t req; // libu write handle
  uv_buf_t buf;   // buffer reference
  uint32_t fd;    // id of the context
} write_req_t;

typedef void *(*register_plugin)();

void PromiseRejectCallback(PromiseRejectMessage message);
void ReportException(Isolate *isolate, TryCatch *try_catch);
bool ExecuteString(Isolate *isolate, Local<String> source, Local<Value> name, bool report_exceptions);
MaybeLocal<String> ReadFile(Isolate *isolate, const char *name);
Local<Context> CreateContext(Isolate *isolate);
// Global Functions
void Print(const FunctionCallbackInfo<Value> &args);
void Version(const FunctionCallbackInfo<Value> &args);
void LoadModule(const FunctionCallbackInfo<Value> &args);
MaybeLocal<Module> OnModuleInstantiate(Local<Context> context, Local<String> specifier, Local<Module> referrer);
void Require(const FunctionCallbackInfo<Value> &args);
void shutdown(uv_loop_t *loop);
void Shutdown(const FunctionCallbackInfo<Value> &args);
void CollectGarbage(const FunctionCallbackInfo<Value> &args);
void EnvVars(const FunctionCallbackInfo<Value> &args);
void OnExit(const FunctionCallbackInfo<Value> &args);
void OnUnhandledRejection(const FunctionCallbackInfo<Value> &args);

inline void DV8_SET_METHOD(v8::Isolate *isolate, v8::Local<v8::Template> recv, const char *name, v8::FunctionCallback callback)
{
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(isolate, callback);
  v8::Local<v8::String> fn_name = v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kInternalized).ToLocalChecked();
  t->SetClassName(fn_name);
  recv->Set(fn_name, t);
}

inline void DV8_SET_PROTOTYPE_METHOD(v8::Isolate *isolate, v8::Local<v8::FunctionTemplate> recv, const char *name, v8::FunctionCallback callback)
{
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Signature> s = v8::Signature::New(isolate, recv);
  v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(isolate, callback, v8::Local<v8::Value>(), s);
  v8::Local<v8::String> fn_name = v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kInternalized).ToLocalChecked();
  t->SetClassName(fn_name);
  recv->PrototypeTemplate()->Set(fn_name, t);
}

inline void DV8_SET_EXPORT(v8::Isolate *isolate, v8::Local<v8::FunctionTemplate> recv, const char *name, v8::Local<v8::Object> exports)
{
  v8::Local<v8::String> class_name = v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kInternalized).ToLocalChecked();
  exports->Set(class_name, recv->GetFunction());
}

inline void DV8_SET_EXPORT_CONSTANT(v8::Isolate *isolate, v8::Local<v8::Value> obj, const char *name, v8::Local<v8::Object> exports)
{
  v8::Local<v8::String> constant_name = v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kInternalized).ToLocalChecked();
  exports->Set(constant_name, obj);
}

inline bool ShouldAbortOnUncaughtException(v8::Isolate *isolate)
{
  fprintf(stderr, "ShouldAbortOnUncaughtException\n");
  return true;
}

inline void OnFatalError(const char *location, const char *message)
{
  if (location) {
    fprintf(stderr, "FATAL ERROR: %s %s\n", location, message);
  }
  else {
    fprintf(stderr, "FATAL ERROR: %s\n", message);
  }
  fflush(stderr);
}

} // namespace dv8
#endif
