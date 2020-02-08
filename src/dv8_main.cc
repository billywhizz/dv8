#include <dv8.h>
#include "builtins.h"

int main(int argc, char *argv[]) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  setvbuf(stderr, nullptr, _IONBF, 0);
  signal(SIGPIPE, SIG_IGN);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
  uv_disable_stdio_inheritance();
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  dv8::builtins::Environment *env = new dv8::builtins::Environment();
  v8::Isolate *isolate = v8::Isolate::New(create_params);
  {
    isolate->SetAbortOnUncaughtExceptionCallback(dv8::ShouldAbortOnUncaughtException);
    isolate->SetFatalErrorHandler(dv8::OnFatalError);
    isolate->SetOOMErrorHandler(dv8::OOMErrorHandler);
    isolate->SetPromiseRejectCallback(dv8::PromiseRejectCallback);
    isolate->SetCaptureStackTraceForUncaughtExceptions(true, 1000, v8::StackTrace::kDetailed);
    isolate->AddGCPrologueCallback(dv8::beforeGCCallback);
    isolate->AddGCEpilogueCallback(dv8::afterGCCallback);
    isolate->AddMicrotasksCompletedCallback(dv8::microTasksCallback);
    isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = dv8::CreateContext(isolate);
    v8::Context::Scope context_scope(context);
    env->AssignToContext(context);
    env->argc = argc;
    env->argv = uv_setup_args(argc, argv);
    v8::Local<v8::Array> arguments = v8::Array::New(isolate);
    for (int i = 0; i < argc; i++) {
      arguments->Set(context, i, v8::String::NewFromUtf8(isolate, argv[i], v8::NewStringType::kNormal, strlen(argv[i])).ToLocalChecked());
    }
    v8::Local<v8::Object> globalInstance = context->Global();
		v8::Local<v8::Value> obj = globalInstance->Get(context, v8::String::NewFromUtf8(isolate, "dv8", v8::NewStringType::kNormal).ToLocalChecked()).ToLocalChecked();
    v8::Local<v8::Object> dv8 = v8::Local<v8::Object>::Cast(obj);
    globalInstance->Set(context, v8::String::NewFromUtf8(isolate, "global", v8::NewStringType::kNormal).ToLocalChecked(), globalInstance);
    dv8->Set(context, v8::String::NewFromUtf8(isolate, "args", v8::NewStringType::kNormal).ToLocalChecked(), arguments);
    dv8::builtins::Buffer::Init(globalInstance);
    dv8::InspectorClient inspector_client(context, true);
    v8::TryCatch try_catch(isolate);
    env->loop = uv_default_loop();
    v8::Local<v8::String> base = v8::String::NewFromUtf8(isolate, src_main_js, v8::NewStringType::kNormal, static_cast<int>(src_main_js_len)).ToLocalChecked();
    v8::ScriptOrigin baseorigin(v8::String::NewFromUtf8(isolate, "main.js", v8::NewStringType::kNormal).ToLocalChecked(), // resource name
      v8::Integer::New(isolate, 0), // line offset
      v8::Integer::New(isolate, 0),  // column offset
      v8::False(isolate), // is shared cross-origin
      v8::Local<v8::Integer>(),  // script id
      v8::Local<v8::Value>(), // source map url
      v8::False(isolate), // is opaque
      v8::False(isolate), // is wasm
      v8::True(isolate)); // is module
    v8::Local<v8::Module> module;
    v8::ScriptCompiler::Source basescript(base, baseorigin);
    if (!v8::ScriptCompiler::CompileModule(isolate, &basescript).ToLocal(&module)) {
      dv8::PrintStackTrace(isolate, try_catch);
      return 1;
    }
    v8::Maybe<bool> ok = module->InstantiateModule(context, dv8::OnModuleInstantiate);
    if (!ok.ToChecked()) {
      dv8::PrintStackTrace(isolate, try_catch);
      return 1;
    }
    v8::MaybeLocal<v8::Value> result = module->Evaluate(context);
		if (result.IsEmpty()) {
      if (try_catch.HasCaught()) {
        dv8::PrintStackTrace(isolate, try_catch);
        return 2;
      }
    }
    v8::platform::PumpMessageLoop(platform.get(), isolate);
    v8::Local<v8::Value> func = globalInstance->Get(context, v8::String::NewFromUtf8(isolate, "onExit", v8::NewStringType::kNormal).ToLocalChecked()).ToLocalChecked();
    if (func->IsFunction()) {
      v8::Local<v8::Function> onExit = v8::Local<v8::Function>::Cast(func);
      v8::Local<v8::Value> argv[0] = { };
      v8::TryCatch try_catch(isolate);
      onExit->Call(context, globalInstance, 0, argv);
      if (try_catch.HasCaught()) {
        dv8::ReportException(isolate, &try_catch);
      }
      v8::platform::PumpMessageLoop(platform.get(), isolate);
    }
    const double kLongIdlePauseInSeconds = 1.0;
    isolate->ContextDisposedNotification();
    isolate->IdleNotificationDeadline(platform->MonotonicallyIncreasingTime() + kLongIdlePauseInSeconds);
    isolate->LowMemoryNotification();
    uv_tty_reset_mode();
  }
  isolate->Dispose();
  delete env;
  delete create_params.array_buffer_allocator;
  isolate = nullptr;
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  platform.reset();
  return 0;
}
