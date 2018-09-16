#include <assert.h>
#include <libplatform/libplatform.h>
#include <v8.h>
#include <unistd.h>

namespace dv8
{

using v8::ArrayBuffer;
using v8::Context;
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

const char *ToCString(const String::Utf8Value &value)
{
  return *value ? *value : "<string conversion failed>";
}

void ReportException(Isolate *isolate, TryCatch *try_catch)
{
  HandleScope handle_scope(isolate);
  String::Utf8Value exception(isolate, try_catch->Exception());
  const char *exception_string = ToCString(exception);
  Local<Message> message = try_catch->Message();
  if (message.IsEmpty())
  {
    fprintf(stderr, "%s\n", exception_string);
  }
  else
  {
    String::Utf8Value filename(isolate,
                               message->GetScriptOrigin().ResourceName());
    Local<Context> context(isolate->GetCurrentContext());
    const char *filename_string = ToCString(filename);
    int linenum = message->GetLineNumber(context).FromJust();
    fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
    String::Utf8Value sourceline(
        isolate, message->GetSourceLine(context).ToLocalChecked());
    const char *sourceline_string = ToCString(sourceline);
    fprintf(stderr, "%s\n", sourceline_string);
    int start = message->GetStartColumn(context).FromJust();
    for (int i = 0; i < start; i++)
    {
      fprintf(stderr, " ");
    }
    int end = message->GetEndColumn(context).FromJust();
    for (int i = start; i < end; i++)
    {
      fprintf(stderr, "^");
    }
    fprintf(stderr, "\n");
    Local<Value> stack_trace_string;
    if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) &&
        stack_trace_string->IsString() &&
        Local<String>::Cast(stack_trace_string)->Length() > 0)
    {
      String::Utf8Value stack_trace(isolate, stack_trace_string);
      const char *stack_trace_string = ToCString(stack_trace);
      fprintf(stderr, "%s\n", stack_trace_string);
    }
  }
}

bool ExecuteString(Isolate *isolate,
                   Local<String> source,
                   Local<Value> name,
                   bool report_exceptions)
{
  HandleScope handle_scope(isolate);
  TryCatch try_catch(isolate);
  ScriptOrigin origin(name);
  Local<Context> context(isolate->GetCurrentContext());
  Local<Script> script;
  if (!Script::Compile(context, source, &origin).ToLocal(&script))
  {
    if (report_exceptions)
      ReportException(isolate, &try_catch);
    return false;
  }
  else
  {
    Local<Value> result;
    if (!script->Run(context).ToLocal(&result))
    {
      assert(try_catch.HasCaught());
      if (report_exceptions)
        ReportException(isolate, &try_catch);
      return false;
    }
    else
    {
      assert(!try_catch.HasCaught());
      return true;
    }
  }
}

void Version(const FunctionCallbackInfo<Value> &args)
{
  args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(),
                                                v8::V8::GetVersion(),
                                                NewStringType::kNormal)
                                .ToLocalChecked());
}

MaybeLocal<String> ReadFile(Isolate *isolate, const char *name)
{
  FILE *file = fopen(name, "rb");
  if (file == NULL)
    return MaybeLocal<String>();

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
      return MaybeLocal<String>();
    }
  }
  fclose(file);
  MaybeLocal<String> result = String::NewFromUtf8(
      isolate, chars, NewStringType::kNormal, static_cast<int>(size));
  delete[] chars;
  return result;
}

void Print(const FunctionCallbackInfo<Value> &args)
{
  bool first = true;
  for (int i = 0; i < args.Length(); i++)
  {
    HandleScope handle_scope(args.GetIsolate());
    if (first)
    {
      first = false;
    }
    else
    {
      printf(" ");
    }
    String::Utf8Value str(args.GetIsolate(), args[i]);
    const char *cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
}

void Sleep(const FunctionCallbackInfo<Value> &args)
{
  int32_t seconds = args[0]->Int32Value(args.GetIsolate()->GetCurrentContext()).ToChecked();
  sleep(seconds);
}

Local<Context> CreateContext(Isolate *isolate)
{
  Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
  global->Set(String::NewFromUtf8(isolate, "version", NewStringType::kNormal)
                  .ToLocalChecked(),
              FunctionTemplate::New(isolate, Version));
  global->Set(String::NewFromUtf8(isolate, "print", NewStringType::kNormal)
                  .ToLocalChecked(),
              FunctionTemplate::New(isolate, Print));
  global->Set(
      String::NewFromUtf8(isolate, "sleep", NewStringType::kNormal)
          .ToLocalChecked(),
      FunctionTemplate::New(isolate, Sleep));
  return Context::New(isolate, NULL, global);
}

} // namespace dv8

int main(int argc, char *argv[])
{
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
  if (argc > 1)
  {
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate *isolate = v8::Isolate::New(create_params);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = dv8::CreateContext(isolate);
    if (context.IsEmpty())
    {
      fprintf(stderr, "Error creating context\n");
      return 1;
    }
    const char *str = argv[1];
    v8::Local<v8::String> file_name =
        v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal)
            .ToLocalChecked();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::String> source;
    if (!dv8::ReadFile(isolate, str).ToLocal(&source))
    {
      fprintf(stderr, "Error reading '%s'\n", str);
      return 1;
    }
    bool success = dv8::ExecuteString(isolate, source, file_name, true);
    if (!success)
      isolate->ThrowException(
          v8::String::NewFromUtf8(
              isolate, "Error executing file", v8::NewStringType::kNormal)
              .ToLocalChecked());
    while (v8::platform::PumpMessageLoop(platform.get(), isolate))
      continue;
    isolate->Exit();
    isolate->Dispose();
    delete create_params.array_buffer_allocator;
  }
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  return 0;
}
