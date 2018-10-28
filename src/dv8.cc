#include <dv8.h>

namespace dv8
{

using dv8::builtins::Environment;
using v8::HeapSpaceStatistics;

using InitializerCallback = void (*)(Local<Object> exports);

void on_handle_close(uv_handle_t *h) {
  free(h);
}

void shutdown(uv_loop_t *loop) {
  uv_walk(loop, [](uv_handle_t *handle, void *arg) {
    fprintf(stderr, "closing [%p] %s in state: %i\n", handle, uv_handle_type_name(handle->type), uv_is_active(handle));
    uv_close(handle, on_handle_close);
  }, NULL);
}

void Shutdown(const FunctionCallbackInfo<Value> &args) {
  shutdown(uv_default_loop());
}

void ReportException(Isolate *isolate, TryCatch *try_catch) {
  HandleScope handle_scope(isolate);
  fprintf(stderr, "exception\n");
  String::Utf8Value exception(isolate, try_catch->Exception());
  const char *exception_string = *exception;
  Local<Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    fprintf(stderr, "%s\n", exception_string);
  }
  else {
    String::Utf8Value filename(isolate, message->GetScriptOrigin().ResourceName());
    Local<Context> context(isolate->GetCurrentContext());
    const char *filename_string = *filename;
    int linenum = message->GetLineNumber(context).FromJust();
    fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
    String::Utf8Value sourceline(isolate, message->GetSourceLine(context).ToLocalChecked());
    const char *sourceline_string = *sourceline;
    fprintf(stderr, "%s\n", sourceline_string);
    int start = message->GetStartColumn(context).FromJust();
    for (int i = 0; i < start; i++) {
      fprintf(stderr, " ");
    }
    int end = message->GetEndColumn(context).FromJust();
    for (int i = start; i < end; i++)
    {
      fprintf(stderr, "^");
    }
    fprintf(stderr, "\n");
    Local<Value> stack_trace_string;
    if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) && stack_trace_string->IsString() && Local<String>::Cast(stack_trace_string)->Length() > 0)
    {
      String::Utf8Value stack_trace(isolate, stack_trace_string);
      const char *stack_trace_string = *stack_trace;
      fprintf(stderr, "%s\n", stack_trace_string);
    }
  }
}

bool ExecuteString(Isolate *isolate, Local<String> source, Local<Value> name, bool report_exceptions)
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
  args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), v8::V8::GetVersion(), NewStringType::kNormal).ToLocalChecked());
}

MaybeLocal<String> ReadFile(Isolate *isolate, const char *name)
{
  FILE *file = fopen(name, "rb");
  if (file == NULL)
  {
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

void Print(const FunctionCallbackInfo<Value> &args)
{
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
    const char *cstr = *str;
    fprintf(stderr, "%s\n", cstr);
    fflush(stderr);
  }
}

void LoadModule(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  String::Utf8Value str(args.GetIsolate(), args[0]);
  const char *module_name = *str;
  char lib_name[128];
  snprintf(lib_name, 128, "/usr/local/lib/%s.so", module_name);
  uv_lib_t lib;
  int success = uv_dlopen(lib_name, &lib);
  Local<Object> exports;
  if (success != 0)
  {
    fprintf(stderr, "uv_dlopen failed: %i\n", success);
    args.GetReturnValue().Set(exports);
    return;
  }
  bool ok = args[1]->ToObject(context).ToLocal(&exports);
  if (!ok)
  {
    fprintf(stderr, "convert args to local failed\n");
    args.GetReturnValue().Set(exports);
    return;
  }
  char register_name[128];
  snprintf(register_name, 128, "_register_%s", module_name);
  void *address;
  success = uv_dlsym(&lib, register_name, &address);
  if (success != 0)
  {
    fprintf(stderr, "uv_dlsym failed: %i\n", success);
    args.GetReturnValue().Set(exports);
    return;
  }
  register_plugin _init = reinterpret_cast<register_plugin>(address);
  auto _register = reinterpret_cast<InitializerCallback>(_init());
  _register(exports);
  args.GetReturnValue().Set(exports);
  uv_dlclose(&lib);
}

MaybeLocal<Module> OnModuleInstantiate(Local<Context> context, Local<String> specifier, Local<Module> referrer)
{
  HandleScope handle_scope(context->GetIsolate());
  return MaybeLocal<Module>();
}

void CollectGarbage(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  isolate->RequestGarbageCollectionForTesting(v8::Isolate::kFullGarbageCollection);
}

void EnvVars(const FunctionCallbackInfo<Value> &args)
{
  //TODO: not thread safe
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  int size = 0;
  while (environ[size])
    size++;
  Local<v8::Array> envarr = v8::Array::New(isolate);
  for (int i = 0; i < size; ++i)
  {
    const char *var = environ[i];
    envarr->Set(i, String::NewFromUtf8(isolate, var, v8::String::kNormalString, strlen(var)));
  }
  args.GetReturnValue().Set(envarr);
}

void OnExit(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(32));
  if (args[0]->IsFunction())
  {
    Local<Function> onExit = Local<Function>::Cast(args[0]);
    env->onExit.Reset(isolate, onExit);
  }
}

void Require(const FunctionCallbackInfo<Value> &args)
{
  HandleScope handle_scope(args.GetIsolate());
  String::Utf8Value str(args.GetIsolate(), args[0]);
  const char *cstr = *str;
  Local<String> source_text;
  if (!ReadFile(args.GetIsolate(), cstr).ToLocal(&source_text))
  {
    fprintf(stderr, "Error reading '%s'\n", cstr);
    return;
  }
  Local<String> fname =
      String::NewFromUtf8(args.GetIsolate(), cstr, NewStringType::kNormal)
          .ToLocalChecked();
  TryCatch try_catch(args.GetIsolate());
  Local<Integer> line_offset;
  Local<Integer> column_offset;
  line_offset = Integer::New(args.GetIsolate(), 0);
  column_offset = Integer::New(args.GetIsolate(), 0);
  ScriptOrigin origin(fname,
                      line_offset,              // line offset
                      column_offset,            // column offset
                      False(args.GetIsolate()), // is cross origin
                      Local<Integer>(),         // script id
                      Local<Value>(),           // source map URL
                      False(args.GetIsolate()), // is opaque (?)
                      False(args.GetIsolate()), // is WASM
                      True(args.GetIsolate())); // is ES6 module
  Local<Context> context(args.GetIsolate()->GetCurrentContext());
  Local<Module> module;
  ScriptCompiler::Source source(source_text, origin);
  if (!ScriptCompiler::CompileModule(args.GetIsolate(), &source).ToLocal(&module))
  {
    ReportException(args.GetIsolate(), &try_catch);
    args.GetReturnValue().Set(Null(args.GetIsolate()));
  }
  else
  {
    //String::Utf8Value utf8string(args.GetIsolate(), source_text);
    Maybe<bool> ok = module->InstantiateModule(context, OnModuleInstantiate);
    if (!ok.ToChecked())
    {
      fprintf(stderr, "instantiate module failed\n");
      args.GetReturnValue().Set(Null(args.GetIsolate()));
      return;
    }
    MaybeLocal<Value> result = module->Evaluate(context);
    args.GetReturnValue().Set(result.ToLocalChecked());
  }
}

Local<Context> CreateContext(Isolate *isolate)
{
  Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
  global->Set(String::NewFromUtf8(isolate, "version", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Version));
  global->Set(String::NewFromUtf8(isolate, "print", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Print));
  global->Set(String::NewFromUtf8(isolate, "module", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, LoadModule));
  global->Set(String::NewFromUtf8(isolate, "require", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Require));
  global->Set(String::NewFromUtf8(isolate, "shutdown", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Shutdown));
  global->Set(String::NewFromUtf8(isolate, "gc", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, CollectGarbage));
  global->Set(String::NewFromUtf8(isolate, "env", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, EnvVars));
  global->Set(String::NewFromUtf8(isolate, "onExit", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, OnExit));
  return Context::New(isolate, NULL, global);
}

} // namespace dv8
