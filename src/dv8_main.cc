#include <dv8.h>

int main(int argc, char *argv[]) {
  if (argc > 1) {
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(argv[0]);
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    const char* flags = "--optimize-for-size --use-strict --max-old-space-size=8 --no-expose-wasm --predictable --single-threaded --single-threaded-gc";
    int flaglen = strlen(flags);
    v8::V8::SetFlagsFromString(flags, flaglen);
    v8::V8::SetFlagsFromCommandLine(&argc, argv, false);
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
      if (context.IsEmpty()) {
        fprintf(stderr, "Error creating context\n");
        return 1;
      }
      env->AssignToContext(context);
      const char *str = argv[1];
      env->loop = uv_default_loop();
      v8::Context::Scope context_scope(context);
      v8::Local<v8::Array> arguments = v8::Array::New(isolate);
      for (int i = 0; i < argc; i++) {
        arguments->Set(i, v8::String::NewFromUtf8(isolate, argv[i], v8::String::kNormalString, strlen(argv[i])));
      }
      v8::Local<v8::Object> globalInstance = context->Global();
      globalInstance->Set(v8::String::NewFromUtf8(isolate, "global", v8::NewStringType::kNormal).ToLocalChecked(), globalInstance);
      globalInstance->Set(v8::String::NewFromUtf8(isolate, "args", v8::NewStringType::kNormal).ToLocalChecked(), arguments);
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
      if (!env->onExit.IsEmpty()) {
        const unsigned int argc = 0;
        v8::Local<v8::Value> argv[argc] = { };
        v8::Local<v8::Function> onExit = v8::Local<v8::Function>::New(isolate, env->onExit);
        v8::TryCatch try_catch(isolate);
        onExit->Call(globalInstance, 0, argv);
        if (try_catch.HasCaught()) {
            dv8::DecorateErrorStack(isolate, try_catch);
        }
      }
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
