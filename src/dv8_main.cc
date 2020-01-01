#include <dv8.h>
#include "builtins.h"

uint64_t start;

int main(int argc, char *argv[]) {
  start = uv_hrtime();
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
  setvbuf(stdout, nullptr, _IONBF, 0);
  setvbuf(stderr, nullptr, _IONBF, 0);

  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8::Isolate *isolate = v8::Isolate::New(create_params);
  {
    isolate->SetAbortOnUncaughtExceptionCallback(dv8::ShouldAbortOnUncaughtException);
    isolate->SetFatalErrorHandler(dv8::OnFatalError);
    isolate->SetOOMErrorHandler(dv8::OOMErrorHandler);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = dv8::CreateContext(isolate);
    v8::Context::Scope context_scope(context);
    context->AllowCodeGenerationFromStrings(true);
    isolate->SetPromiseRejectCallback(dv8::PromiseRejectCallback);
    isolate->SetCaptureStackTraceForUncaughtExceptions(true, 1000, v8::StackTrace::kDetailed);
    dv8::builtins::Environment *env = new dv8::builtins::Environment();
    env->AssignToContext(context);
    env->loop = uv_default_loop();
    env->error = (dv8::js_error*)calloc(sizeof(dv8::js_error), 1);
    env->error->hasError = 0;
    v8::Local<v8::Array> arguments = v8::Array::New(isolate);
    for (int i = 0; i < argc; i++) {
      arguments->Set(context, i, v8::String::NewFromUtf8(isolate, argv[i], v8::NewStringType::kNormal, strlen(argv[i])).ToLocalChecked());
    }
    v8::Local<v8::Object> globalInstance = context->Global();
    globalInstance->Set(context, v8::String::NewFromUtf8(isolate, "global", v8::NewStringType::kNormal).ToLocalChecked(), globalInstance);
    globalInstance->Set(context, v8::String::NewFromUtf8(isolate, "args", v8::NewStringType::kNormal).ToLocalChecked(), arguments);
    dv8::builtins::Buffer::Init(globalInstance);
    dv8::InspectorClient inspector_client(context, true);

    v8::TryCatch try_catch(isolate);
    v8::MaybeLocal<v8::String> base;
     const char* base_name;
    if (argc == 3 && strcmp("-e", argv[1]) == 0) {
      base = v8::String::NewFromUtf8(isolate, src_base_js, v8::NewStringType::kNormal, static_cast<int>(src_base_js_len));
      base_name = "base.js";
    } else {
      base = v8::String::NewFromUtf8(isolate, src_main_js, v8::NewStringType::kNormal, static_cast<int>(src_main_js_len));
      base_name = "main.js";
    }
   // https://v8docs.nodesource.com/node-10.6/db/d84/classv8_1_1_script_origin.html
    v8::ScriptOrigin baseorigin(v8::String::NewFromUtf8(isolate, base_name, v8::NewStringType::kNormal).ToLocalChecked(), // resource name
      v8::Integer::New(isolate, 0), // line offset
      v8::Integer::New(isolate, 0),  // column offset
      v8::False(isolate), // is shared cross-origin
      v8::Local<v8::Integer>(),  // script id
      v8::Local<v8::Value>(), // source map url
      v8::False(isolate), // is opaque
      v8::False(isolate), // is wasm
      v8::True(isolate)); // is module
    v8::Local<v8::Module> module;
    v8::ScriptCompiler::Source basescript(base.ToLocalChecked(), baseorigin);
    if (!v8::ScriptCompiler::CompileModule(isolate, &basescript).ToLocal(&module)) {
      dv8::ReportException(isolate, &try_catch);
      return 1;
    }
    v8::Maybe<bool> ok = module->InstantiateModule(context, dv8::OnModuleInstantiate);
    if (!ok.ToChecked()) {
      dv8::ReportException(isolate, &try_catch);
      return 1;
    }
    uint64_t now = uv_hrtime();
    module->Evaluate(context);
    if (try_catch.HasCaught()) {
      dv8::ReportException(isolate, &try_catch);
      return 1;
    }
    v8::platform::PumpMessageLoop(platform.get(), isolate);
    dv8::shutdown(uv_default_loop());
    uv_tty_reset_mode();
    int r = uv_loop_close(uv_default_loop());
    if (r != 0) {
      //fprintf(stderr, "uv_loop_close: %i\n", r);
    }
  }
  // cleanup
  //isolate->Dispose();
  delete create_params.array_buffer_allocator;
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  return 0;
}
