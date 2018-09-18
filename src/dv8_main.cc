#include <dv8.h>

int main(int argc, char *argv[]) {
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
  setvbuf(stdout, nullptr, _IONBF, 0);
  setvbuf(stderr, nullptr, _IONBF, 0);
  dv8::SingnalHandler();
  if (argc > 1) {
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate *isolate = v8::Isolate::New(create_params);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = dv8::CreateContext(isolate);
    if (context.IsEmpty()) {
      fprintf(stderr, "Error creating context\n");
      return 1;
    }
    const char *str = argv[1];
    v8::Local<v8::String> file_name = v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal).ToLocalChecked();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Object> globalInstance = context->Global();
    globalInstance->Set(v8::String::NewFromUtf8(isolate, "global", v8::NewStringType::kNormal).ToLocalChecked(), globalInstance);
    dv8::builtins::Buffer::Init(globalInstance);
    v8::Local<v8::String> source;
    if (!dv8::ReadFile(isolate, str).ToLocal(&source)) {
      fprintf(stderr, "Error reading '%s'\n", str);
      return 1;
    }
    bool success = dv8::ExecuteString(isolate, source, file_name, true);
    if (!success) isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Error executing file", v8::NewStringType::kNormal).ToLocalChecked());
    while (v8::platform::PumpMessageLoop(platform.get(), isolate)) continue;
    bool more;
    do {
      uv_run(uv_default_loop(), UV_RUN_DEFAULT);
      more = uv_loop_alive(uv_default_loop());
      fprintf(stderr, "more: %i\n", more);
      dv8::shutdown();
      if (more)
        continue;
      more = uv_loop_alive(uv_default_loop());
      fprintf(stderr, "more: %i\n", more);
    } while (more == true);
    int r = uv_loop_close(uv_default_loop());
    fprintf(stderr, "uv_loop_close: %i\n", r);
    isolate->Exit();
    isolate->Dispose();
    delete create_params.array_buffer_allocator;
  }
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  return 0;
}
