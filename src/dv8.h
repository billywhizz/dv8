#ifndef DV8_H
#define DV8_H

#include <assert.h>
#include <unistd.h>
#include <v8.h>
#include <libplatform/libplatform.h>
#include <v8-inspector.h>
#include <common.h>
#include <buffer.h>
#include <env.h>
#include <string.h>
#include <modules.h>
#include <sys/utsname.h>
#include <gnu/libc-version.h>
#include <time.h>
#include <limits.h>
#include <jsys.h>
#include <sys/wait.h>

//#include <execinfo.h>
//#include <cxxabi.h>
//#include <dlfcn.h>

#define MICROS_PER_SEC 1e6
#define SO_NOSIGPIPE 1
#ifndef V8_DLOPEN
  #define V8_DLOPEN 1
#endif
#ifndef TRACE
  #define TRACE 0
#endif

template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];

#define arraysize(array) (sizeof(ArraySizeHelper(array)))

extern char **environ;
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
using dv8::builtins::Environment;
using v8::HeapSpaceStatistics;
using v8::Exception;
using v8::Promise;
using v8::Global;
using v8::Array;
using v8::BigUint64Array;
using v8_inspector::V8InspectorClient;
using v8_inspector::V8Inspector;
using v8_inspector::StringBuffer;
using v8_inspector::StringView;
using v8_inspector::V8ContextInfo;
using v8_inspector::V8InspectorSession;
using v8_inspector::V8Inspector;
using v8::Platform;
using v8::Int32Array;

class SymbolInfo {
  public:
  std::string name;
  std::string filename;
  size_t line = 0;
  size_t dis = 0;
};

class InspectorFrontend final : public V8Inspector::Channel {
 public:

  explicit InspectorFrontend(Local<Context> context) {
    isolate_ = context->GetIsolate();
    context_.Reset(isolate_, context);
  }

  ~InspectorFrontend() override = default;

 private:

  void sendResponse(int callId, std::unique_ptr<StringBuffer> message) override {
    Send(message->string());
  }

  void sendNotification(std::unique_ptr<StringBuffer> message) override {
    Send(message->string());
  }

  void flushProtocolNotifications() override {}

  void Send(const v8_inspector::StringView& string) {
    v8::Isolate::AllowJavascriptExecutionScope allow_script(isolate_);
    int length = static_cast<int>(string.length());
    Local<String> message = (string.is8Bit() ? v8::String::NewFromOneByte(isolate_, reinterpret_cast<const uint8_t*>(string.characters8()), v8::NewStringType::kNormal, length) : v8::String::NewFromTwoByte(isolate_, reinterpret_cast<const uint16_t*>(string.characters16()), v8::NewStringType::kNormal, length)).ToLocalChecked();
    Local<String> callback_name = v8::String::NewFromUtf8(isolate_, "receive", v8::NewStringType::kNormal).ToLocalChecked();
    Local<Context> context = context_.Get(isolate_);
    Local<Value> callback = context->Global()->Get(context, callback_name).ToLocalChecked();
    if (callback->IsFunction()) {
      v8::TryCatch try_catch(isolate_);
      Local<Value> args[] = {message};
      Local<Value> ret = Local<Function>::Cast(callback)->Call(context, Undefined(isolate_), 1, args).ToLocalChecked();
      if (ret->IsNull()) {

      }
    }
  }

  Isolate* isolate_;
  Global<Context> context_;
};

class InspectorClient : public V8InspectorClient {
 public:
  InspectorClient(Local<Context> context, bool connect) {
    if (!connect) return;
    isolate_ = context->GetIsolate();
    channel_.reset(new InspectorFrontend(context));
    inspector_ = V8Inspector::create(isolate_, this);
    session_ = inspector_->connect(1, channel_.get(), StringView());
    context->SetAlignedPointerInEmbedderData(kInspectorClientIndex, this);
    inspector_->contextCreated(V8ContextInfo(context, kContextGroupId, StringView()));
    Local<Value> function = FunctionTemplate::New(isolate_, SendInspectorMessage)->GetFunction(context).ToLocalChecked();
    Local<String> function_name = String::NewFromUtf8(isolate_, "send", NewStringType::kNormal).ToLocalChecked();
    context->Global()->Set(context, function_name, function).FromJust();
    context_.Reset(isolate_, context);
  }

  void runMessageLoopOnPause(int context_group_id) override {
    Local<String> callback_name = v8::String::NewFromUtf8(isolate_, "onRunMessageLoop", v8::NewStringType::kNormal).ToLocalChecked();
    Local<Context> context = context_.Get(isolate_);
    Local<Value> callback = context->Global()->Get(context, callback_name).ToLocalChecked();
    if (callback->IsFunction()) {
      v8::TryCatch try_catch(isolate_);
      Local<Value> args[] = {};
      Local<Value> ret = Local<Function>::Cast(callback)->Call(context, Undefined(isolate_), 0, args).ToLocalChecked();
      if (ret->IsNull()) {

      }
    }
  }

  void quitMessageLoopOnPause() override {
    Local<String> callback_name = v8::String::NewFromUtf8(isolate_, "onQuitMessageLoop", v8::NewStringType::kNormal).ToLocalChecked();
    Local<Context> context = context_.Get(isolate_);
    Local<Value> callback = context->Global()->Get(context, callback_name).ToLocalChecked();
    if (callback->IsFunction()) {
      v8::TryCatch try_catch(isolate_);
      Local<Value> args[] = {};
      Local<Value> ret = Local<Function>::Cast(callback)->Call(context, Undefined(isolate_), 0, args).ToLocalChecked();
      if (ret->IsNull()) {

      }
    }
  }

 private:

  static V8InspectorSession* GetSession(Local<Context> context) {
    InspectorClient* inspector_client = static_cast<InspectorClient*>(context->GetAlignedPointerFromEmbedderData(kInspectorClientIndex));
    return inspector_client->session_.get();
  }

  Local<Context> ensureDefaultContextInGroup(int group_id) override {
    return context_.Get(isolate_);
  }

  static void SendInspectorMessage(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    Local<Context> context = isolate->GetCurrentContext();
    args.GetReturnValue().Set(Undefined(isolate));
    Local<String> message = args[0]->ToString(context).ToLocalChecked();
    V8InspectorSession* session = InspectorClient::GetSession(context);
    int length = message->Length();
    std::unique_ptr<uint16_t[]> buffer(new uint16_t[length]);
    message->Write(isolate, buffer.get(), 0, length);
    StringView message_view(buffer.get(), length);
    session->dispatchProtocolMessage(message_view);
    args.GetReturnValue().Set(True(isolate));
  }

  static const int kContextGroupId = 1;
  std::unique_ptr<V8Inspector> inspector_;
  std::unique_ptr<V8InspectorSession> session_;
  std::unique_ptr<V8Inspector::Channel> channel_;
  Global<Context> context_;
  Isolate* isolate_;
};

void PromiseRejectCallback(PromiseRejectMessage message);
void ReportException(Isolate *isolate, TryCatch *try_catch);
Local<Context> CreateContext(Isolate *isolate);
// DV8 Runtime Functions
void Print(const FunctionCallbackInfo<Value> &args);
void Err(const FunctionCallbackInfo<Value> &args);
MaybeLocal<Module> OnModuleInstantiate(Local<Context> context, Local<String> specifier, Local<Module> referrer);
void shutdown(jsys_loop* loop);
void Shutdown(const FunctionCallbackInfo<Value> &args);
void MemoryUsage(const FunctionCallbackInfo<Value> &args);
void Exit(const FunctionCallbackInfo<Value> &args);
void HeapSpaceUsage(const FunctionCallbackInfo<Value> &args);
void EnvVars(const FunctionCallbackInfo<Value> &args);
void RunScript(const FunctionCallbackInfo<Value> &args);
void CompileScript(const FunctionCallbackInfo<Value> &args);
void RunModule(const FunctionCallbackInfo<Value> &args);
void Cwd(const FunctionCallbackInfo<Value> &args);
void Spawn(const FunctionCallbackInfo<Value> &args);
void WaitPID(const FunctionCallbackInfo<Value> &args);
void Sleep(const FunctionCallbackInfo<Value> &args);
void USleep(const FunctionCallbackInfo<Value> &args);
void NanoSleep(const FunctionCallbackInfo<Value> &args);
void PrintStackTrace(v8::Isolate* isolate, const v8::TryCatch& try_catch);
void shutdown(jsys_loop* loop, int rc);
void beforeGCCallback(v8::Isolate* isolate, v8::GCType type, v8::GCCallbackFlags flags);
void afterGCCallback(v8::Isolate* isolate, v8::GCType type, v8::GCCallbackFlags flags);

inline void beforeGCCallback(v8::Isolate* isolate, v8::GCType type, v8::GCCallbackFlags flags) {
  //fprintf(stderr, "before GC\n");
}

inline void afterGCCallback(v8::Isolate* isolate, v8::GCType type, v8::GCCallbackFlags flags) {
  //fprintf(stderr, "after GC\n");
}

inline void microTasksCallback(v8::Isolate* isolate) {
  //fprintf(stderr, "on MicroTasks\n");
}

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
  v8::Local<v8::String> class_name = v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kInternalized).ToLocalChecked();
  Local<Context> context = isolate->GetCurrentContext();
  exports->Set(context, class_name, recv->GetFunction(context).ToLocalChecked());
}

inline void DV8_SET_EXPORT_CONSTANT(v8::Isolate *isolate, v8::Local<v8::Value> obj, const char *name, v8::Local<v8::Object> exports) {
  v8::Local<v8::String> constant_name = v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kInternalized).ToLocalChecked();
  Local<Context> context = isolate->GetCurrentContext();
  exports->Set(context, constant_name, obj);
}

inline void DV8_SET_CONSTANT(v8::Isolate *isolate, v8::Local<v8::Value> obj, const char *name, v8::Local<v8::Template> recv) {
  v8::Local<v8::String> constant_name = v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kInternalized).ToLocalChecked();
  recv->Set(constant_name, obj);
}

inline bool ShouldAbortOnUncaughtException(v8::Isolate *isolate) {
  fprintf(stderr, "ShouldAbortOnUncaughtException\n");
  return true;
}
/*
inline dv8::SymbolInfo LookupSymbol(void* address) {
  Dl_info info;
  const bool have_info = dladdr(address, &info);
  SymbolInfo ret;
  if (!have_info)
    return ret;

  if (info.dli_sname != nullptr) {
    if (char* demangled =
            abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, nullptr)) {
      ret.name = demangled;
      free(demangled);
    } else {
      ret.name = info.dli_sname;
    }
  }

  if (info.dli_fname != nullptr) {
    ret.filename = info.dli_fname;
  }

  return ret;
}

inline void dumpErrorInfo() {
  // todo: get this working. dump version, loop handles etc.
  void* frames[256];
  const int size = backtrace(frames, arraysize(frames));
  fprintf(stderr, "backtrace: %i\n", size);
  for (int i = 0; i < size; i += 1) {
    void* frame = frames[i];
    dv8::SymbolInfo s = LookupSymbol(frame);
    fprintf(stderr, "%2d: %p %s + [%s]\n", i, frame, s.name.c_str(), s.filename.c_str());
  }
}
*/
inline void OnFatalError(const char *location, const char *message) {
  fprintf(stderr, "FATAL ERROR: %s %s\n", location, message);
  //dumpErrorInfo();
  fflush(stderr);
}

inline void OOMErrorHandler(const char *location, bool is_heap_oom) {
  fprintf(stderr, "OOM ERROR: %s %i\n", location, is_heap_oom);
  //dumpErrorInfo();
  fflush(stderr);
}

} // namespace dv8
#endif
