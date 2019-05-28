#include <dv8.h>
#include "builtins.h"

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    fprintf(stderr, "Usage: dv8 script.js\n");
    return 1;
  }
  // initialize platform and parse v8 command line switches
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
  // set the default v8 options for runtime
  //const char *flags = "--optimize-for-size --use-strict --max-old-space-size=8 --no-expose-wasm --predictable --single-threaded --single-threaded-gc";
  //int flaglen = strlen(flags);
  //v8::V8::SetFlagsFromString(flags, flaglen);
  // Disable stdio buffering, it interacts poorly with printf()
  // calls elsewhere in the program (e.g., any logging from V8.)
  setvbuf(stdout, nullptr, _IONBF, 0);
  setvbuf(stderr, nullptr, _IONBF, 0);
  // options for isolate
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8::Isolate *isolate = v8::Isolate::New(create_params);
  {
    // initialize the isolate
    isolate->SetAbortOnUncaughtExceptionCallback(dv8::ShouldAbortOnUncaughtException);
    isolate->SetFatalErrorHandler(dv8::OnFatalError);
    isolate->SetOOMErrorHandler(dv8::OOMErrorHandler);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    // create the context
    v8::Local<v8::Context> context = dv8::CreateContext(isolate);
    v8::Context::Scope context_scope(context);
    // disable this to disallow eval etc.
    context->AllowCodeGenerationFromStrings(true);
    isolate->SetPromiseRejectCallback(dv8::PromiseRejectCallback);
    // initialise an environment with reference to event loop for the context
    dv8::builtins::Environment *env = new dv8::builtins::Environment();
    env->AssignToContext(context);
    env->loop = uv_default_loop();
    env->error = (dv8::js_error*)calloc(sizeof(dv8::js_error), 1);
    env->error->hasError = 0;
    // store the command line arguments
    v8::Local<v8::Array> arguments = v8::Array::New(isolate);
    for (int i = 0; i < argc; i++) {
      arguments->Set(i, v8::String::NewFromUtf8(isolate, argv[i], v8::String::kNormalString, strlen(argv[i])));
    }
    // set up global
    v8::Local<v8::Object> globalInstance = context->Global();
    globalInstance->Set(v8::String::NewFromUtf8(isolate, "global", v8::NewStringType::kNormal).ToLocalChecked(), globalInstance);
    globalInstance->Set(v8::String::NewFromUtf8(isolate, "args", v8::NewStringType::kNormal).ToLocalChecked(), arguments);
    dv8::builtins::Buffer::Init(globalInstance);
    dv8::InspectorClient inspector_client(context, true);
    v8::TryCatch try_catch(isolate);
    // compile and run the base module
    v8::MaybeLocal<v8::String> base = v8::String::NewFromUtf8(isolate, src_base_js, v8::NewStringType::kNormal, static_cast<int>(src_base_js_len));
    // https://v8docs.nodesource.com/node-10.6/db/d84/classv8_1_1_script_origin.html
    v8::ScriptOrigin baseorigin(v8::String::NewFromUtf8(isolate, "./lib/base.js", v8::NewStringType::kNormal).ToLocalChecked(), // resource name
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
    module->Evaluate(context);
    v8::platform::PumpMessageLoop(platform.get(), isolate);
    dv8::shutdown(uv_default_loop());
    uv_tty_reset_mode();
    int r = uv_loop_close(uv_default_loop());
    if (r != 0) {
      fprintf(stderr, "uv_loop_close: %i\n", r);
    }
  }
  // cleanup
  isolate->Dispose();
  delete create_params.array_buffer_allocator;
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  return 0;
}
