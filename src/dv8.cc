#include <dv8.h>

namespace dv8 {

using InitializerCallback = void (*)(Local<Object> exports);

const char *ToCString(const String::Utf8Value &value) {
  return *value ? *value : "<string conversion failed>";
}

void ReportException(Isolate *isolate, TryCatch *try_catch) {
  HandleScope handle_scope(isolate);
fprintf(stderr, "exception\n");
  String::Utf8Value exception(isolate, try_catch->Exception());
  const char *exception_string = ToCString(exception);
  Local<Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    fprintf(stderr, "%s\n", exception_string);
  }
  else {
    String::Utf8Value filename(isolate, message->GetScriptOrigin().ResourceName());
    Local<Context> context(isolate->GetCurrentContext());
    const char *filename_string = ToCString(filename);
    int linenum = message->GetLineNumber(context).FromJust();
    fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
    String::Utf8Value sourceline(isolate, message->GetSourceLine(context).ToLocalChecked());
    const char *sourceline_string = ToCString(sourceline);
    fprintf(stderr, "%s\n", sourceline_string);
    int start = message->GetStartColumn(context).FromJust();
    for (int i = 0; i < start; i++) {
      fprintf(stderr, " ");
    }
    int end = message->GetEndColumn(context).FromJust();
    for (int i = start; i < end; i++) {
      fprintf(stderr, "^");
    }
    fprintf(stderr, "\n");
    Local<Value> stack_trace_string;
    if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) && stack_trace_string->IsString() && Local<String>::Cast(stack_trace_string)->Length() > 0) {
      String::Utf8Value stack_trace(isolate, stack_trace_string);
      const char *stack_trace_string = ToCString(stack_trace);
      fprintf(stderr, "%s\n", stack_trace_string);
    }
  }
}

bool ExecuteString(Isolate *isolate, Local<String> source, Local<Value> name, bool report_exceptions) {
  HandleScope handle_scope(isolate);
  TryCatch try_catch(isolate);
  ScriptOrigin origin(name);
  Local<Context> context(isolate->GetCurrentContext());
  Local<Script> script;
  if (!Script::Compile(context, source, &origin).ToLocal(&script))
  {
    if (report_exceptions) ReportException(isolate, &try_catch);
    return false;
  }
  else
  {
    Local<Value> result;
    if (!script->Run(context).ToLocal(&result))
    {
      assert(try_catch.HasCaught());
      if (report_exceptions) ReportException(isolate, &try_catch);
      return false;
    }
    else
    {
      assert(!try_catch.HasCaught());
      return true;
    }
  }
}

void Version(const FunctionCallbackInfo<Value> &args) {
  args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), v8::V8::GetVersion(), NewStringType::kNormal).ToLocalChecked());
}

MaybeLocal<String> ReadFile(Isolate *isolate, const char *name) {
  FILE *file = fopen(name, "rb");
  if (file == NULL) return MaybeLocal<String>();
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
  MaybeLocal<String> result = String::NewFromUtf8(isolate, chars, NewStringType::kNormal, static_cast<int>(size));
  delete[] chars;
  return result;
}

void Print(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  bool first = true;
  for (int i = 0; i < args.Length(); i++)
  {
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

void LoadModule(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  String::Utf8Value str(args.GetIsolate(), args[0]);
  const char *module_name = ToCString(str);
  char lib_name[128];
  snprintf(lib_name, 128, "./%s.so", module_name);
  uv_lib_t lib;
  int success = uv_dlopen(lib_name, &lib);
  Local<Object> exports;
  bool r = args[1]->ToObject(context).ToLocal(&exports);
  char register_name[128];
  snprintf(register_name, 128, "_register_%s", module_name);
  void *address;
  success = uv_dlsym(&lib, register_name, &address);
  register_plugin _init = reinterpret_cast<register_plugin>(address);
  auto _register = reinterpret_cast<InitializerCallback>(_init());
  _register(exports);
  args.GetReturnValue().Set(exports);
}

Local<Context> CreateContext(Isolate *isolate) {
  Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
  global->Set(String::NewFromUtf8(isolate, "version", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Version));
  global->Set(String::NewFromUtf8(isolate, "print", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Print));
  global->Set(String::NewFromUtf8(isolate, "module", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, LoadModule));
  return Context::New(isolate, NULL, global);
}

} // namespace dv8
