#include <dv8.h>

bool ShouldAbortOnUncaughtException(v8::Isolate *isolate) {
  fprintf(stderr, "ShouldAbortOnUncaughtException\n");
  return true;
}

static void OnFatalError(const char *location, const char *message) {
  if (location) {
    fprintf(stderr, "FATAL ERROR: %s %s\n", location, message);
  }
  else {
    fprintf(stderr, "FATAL ERROR: %s\n", message);
  }
  fflush(stderr);
}

int main(int argc, char *argv[]) {
  if (argc > 1) {
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(argv[0]);
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
    dv8::SingnalHandler();
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate *isolate = v8::Isolate::New(create_params);
    {
      isolate->SetAbortOnUncaughtExceptionCallback(ShouldAbortOnUncaughtException);
      isolate->SetFatalErrorHandler(OnFatalError);
      v8::Isolate::Scope isolate_scope(isolate);
      v8::HandleScope handle_scope(isolate);
      v8::Local<v8::Context> context = dv8::CreateContext(isolate);
      if (context.IsEmpty()) {
        fprintf(stderr, "Error creating context\n");
        return 1;
      }
      const char *str = argv[1];
      v8::Context::Scope context_scope(context);
      v8::Local<v8::Object> globalInstance = context->Global();
      globalInstance->Set(v8::String::NewFromUtf8(isolate, "global", v8::NewStringType::kNormal).ToLocalChecked(), globalInstance);
      dv8::builtins::Buffer::Init(globalInstance);
      dv8::builtins::Timer::Init(globalInstance);
      dv8::builtins::TTY::Init(globalInstance);
      v8::TryCatch try_catch(isolate);
      v8::MaybeLocal<v8::String> source = dv8::ReadFile(isolate, str);
      if (try_catch.HasCaught()) {
        dv8::DecorateErrorStack(isolate, try_catch);
        return 1;
      }
      v8::MaybeLocal<v8::Script> script = v8::Script::Compile(context, source.ToLocalChecked());
      if (try_catch.HasCaught()) {
        dv8::DecorateErrorStack(isolate, try_catch);
        return 1;
      }
      script.ToLocalChecked()->Run(context);
      if (try_catch.HasCaught()) {
        dv8::DecorateErrorStack(isolate, try_catch);
        return 1;
      }
      bool more;
      do {
        uv_run(uv_default_loop(), UV_RUN_DEFAULT);
        more = uv_loop_alive(uv_default_loop());
        if (more) {
          continue;
        }
      } while (more == true);
      int r = uv_loop_close(uv_default_loop());
      fprintf(stderr, "uv_loop_close: %i\n", r);
    }
    //isolate->Exit();
    isolate->Dispose();
    delete create_params.array_buffer_allocator;
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    return 0;
  }
  return 1;
}
