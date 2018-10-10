#include <dv8.h>

int main(int argc, char *argv[]) {
  if (argc > 1) {
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(argv[0]);
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
      v8::Isolate::Scope isolate_scope(isolate);
      v8::HandleScope handle_scope(isolate);
      v8::Local<v8::Context> context = dv8::CreateContext(isolate);
      dv8::builtins::Environment* env = new dv8::builtins::Environment();
      env->AssignToContext(context);
      if (context.IsEmpty()) {
        fprintf(stderr, "Error creating context\n");
        return 1;
      }
      const char *str = argv[1];
      env->loop = uv_default_loop();
      v8::Context::Scope context_scope(context);
      v8::Local<v8::Object> globalInstance = context->Global();
      globalInstance->Set(v8::String::NewFromUtf8(isolate, "global", v8::NewStringType::kNormal).ToLocalChecked(), globalInstance);
      dv8::builtins::Buffer::Init(globalInstance);
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
      int alive;
      do {
        uv_run(uv_default_loop(), UV_RUN_DEFAULT);
        alive = uv_loop_alive(uv_default_loop());
        if (alive != 0) {
          continue;
        }
        alive = uv_loop_alive(uv_default_loop());
      } while (alive != 0);
      dv8::shutdown(uv_default_loop());
      uv_tty_reset_mode();
      int r = uv_loop_close(uv_default_loop());
      if (r != 0) {
        fprintf(stderr, "uv_loop_close: %i\n", r);
      }
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
