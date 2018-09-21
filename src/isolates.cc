#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <v8.h>
#include <libplatform/libplatform.h>
#include <uv.h>
#include <timer.h>
#include <buffer.h>
#include <execinfo.h>
#include <common.h>

void Log(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  v8::String::Utf8Value str(isolate, args[0]);
  fprintf(stderr, "%s\n", *str);
  fflush(stderr);
}

v8::MaybeLocal<v8::String> ReadFile(v8::Isolate *isolate, const char *name)
{
  FILE *file = fopen(name, "rb");
  if (file == NULL) {
    isolate->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(isolate, "Bad File", v8::NewStringType::kNormal).ToLocalChecked()));
    return v8::MaybeLocal<v8::String>();
  }
  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  rewind(file);
  char *chars = new char[size + 1];
  chars[size] = '\0';
  for (size_t i = 0; i < size;)
  {
    i += fread(&chars[i], 1, size - i, file);
    if (ferror(file))
    {
      fclose(file);
      isolate->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(isolate, "Read Error", v8::NewStringType::kNormal).ToLocalChecked()));
      return v8::MaybeLocal<v8::String>();
    }
  }
  fclose(file);
  v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(isolate, chars, v8::NewStringType::kNormal, static_cast<int>(size));
  delete[] chars;
  return result;
}

bool ShouldAbortOnUncaughtException(v8::Isolate *isolate)
{
  fprintf(stderr, "ShouldAbortOnUncaughtException\n");
  v8::HandleScope scope(isolate);
  return true;
}

void DumpBacktrace(FILE *fp)
{
  void *array[10];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace(array, 10);
  strings = backtrace_symbols(array, size);

  fprintf(fp, "Obtained %zd stack frames.\n", size);

  for (i = 0; i < size; i++)
    fprintf(fp, "%s\n", strings[i]);

  free(strings);
}

static void OnFatalError(const char *location, const char *message)
{
  if (location)
  {
    fprintf(stderr, "FATAL ERROR: %s %s\n", location, message);
  }
  else
  {
    fprintf(stderr, "FATAL ERROR: %s\n", message);
  }
  //DumpBacktrace(stderr);
  fflush(stderr);
}

int main(int argc, char *argv[])
{
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
  setvbuf(stdout, nullptr, _IONBF, 0);
  setvbuf(stderr, nullptr, _IONBF, 0);
  //dv8::SingnalHandler();
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8::Isolate *isolate = v8::Isolate::New(create_params);
  {
    isolate->SetAbortOnUncaughtExceptionCallback(ShouldAbortOnUncaughtException);
    isolate->SetFatalErrorHandler(OnFatalError);
    v8::Isolate::Scope isolate_scope(isolate);
    const char *filename = argv[1];
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
    global->Set(v8::String::NewFromUtf8(isolate, "log", v8::NewStringType::kNormal).ToLocalChecked(), v8::FunctionTemplate::New(isolate, Log));
    v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Object> globalInstance = context->Global();
    globalInstance->Set(v8::String::NewFromUtf8(isolate, "global", v8::NewStringType::kNormal).ToLocalChecked(), globalInstance);
    dv8::builtins::Timer::Init(globalInstance);
    dv8::builtins::Buffer::Init(globalInstance);
    v8::TryCatch try_catch(isolate);
    v8::MaybeLocal<v8::String> source = ReadFile(isolate, filename);
    if (try_catch.HasCaught()) {
      dv8::DecorateErrorStack(isolate, try_catch);
      return 1;
    }
    v8::MaybeLocal<v8::Script> script = v8::Script::Compile(context, source.ToLocalChecked());
    if (try_catch.HasCaught()) {
      dv8::DecorateErrorStack(isolate, try_catch);
      return 1;
    }
    v8::MaybeLocal<v8::Value> result = script.ToLocalChecked()->Run(context);
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
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete create_params.array_buffer_allocator;
  return 0;
}
