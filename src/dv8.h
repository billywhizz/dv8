#ifndef DV8_H
#define DV8_H

#include <assert.h>
#include <unistd.h>
#include <v8.h>
#include <libplatform/libplatform.h>
#include <uv.h>
#include <common.h>
#include <buffer.h>
#include <timer.h>

namespace dv8 {

using v8::ArrayBuffer;
using v8::Context;
using v8::Object;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Message;
using v8::NewStringType;
using v8::ObjectTemplate;
using v8::Script;
using v8::ScriptOrigin;
using v8::String;
using v8::TryCatch;
using v8::V8;
using v8::Value;
using v8::Integer;
using v8::Module;
using v8::ScriptCompiler;
using v8::Maybe;
using v8::Function;


static uv_signal_t* signalHandle;

typedef void* (*register_plugin)();

const char *ToCString(const String::Utf8Value &value);
void ReportException(Isolate *isolate, TryCatch *try_catch);
bool ExecuteString(Isolate *isolate, Local<String> source, Local<Value> name, bool report_exceptions);
MaybeLocal<String> ReadFile(Isolate *isolate, const char *name);
Local<Context> CreateContext(Isolate *isolate);
// Global Functions
void Print(const FunctionCallbackInfo<Value> &args);
void Version(const FunctionCallbackInfo<Value> &args);
void LoadModule(const FunctionCallbackInfo<Value>& args);
MaybeLocal<Module> OnModuleInstantiate(Local<Context> context, Local<String> specifier, Local<Module> referrer);
void Require(const FunctionCallbackInfo<Value> &args);
void SingnalHandler();
void OnSignal(uv_signal_t* handle, int signum);
void on_signal_close(uv_handle_t* h);
void shutdown();
void Shutdown(const FunctionCallbackInfo<Value> &args);

inline void DV8_SET_METHOD(v8::Isolate *isolate, v8::Local<v8::Template> recv, const char *name, v8::FunctionCallback callback) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(isolate, callback);
    v8::Local<v8::String> fn_name = v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kInternalized).ToLocalChecked();
    t->SetClassName(fn_name);
    recv->Set(fn_name, t);
}

inline void DV8_SET_PROTOTYPE_METHOD(v8::Isolate *isolate, v8::Local<v8::FunctionTemplate> recv, const char *name, v8::FunctionCallback callback) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Signature> s = v8::Signature::New(isolate, recv);
    v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(isolate, callback, v8::Local<v8::Value>(), s);
    v8::Local<v8::String> fn_name = v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kInternalized).ToLocalChecked();
    t->SetClassName(fn_name);
    recv->PrototypeTemplate()->Set(fn_name, t);
}

inline void DV8_SET_EXPORT(v8::Isolate *isolate, v8::Local<v8::FunctionTemplate> recv, const char *name, v8::Local<v8::Object> exports) {
    v8::Local<v8::String> export_name = v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kInternalized).ToLocalChecked();
    exports->Set(export_name, recv->GetFunction());
}

} // namespace dv8
#endif
