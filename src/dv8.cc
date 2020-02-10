#include <dv8.h>

namespace dv8 {

void PromiseRejectCallback(PromiseRejectMessage message) {
  Local<Promise> promise = message.GetPromise();
  Isolate* isolate = promise->GetIsolate();
  HandleScope handle_scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  PromiseRejectEvent event = message.GetEvent();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
  Local<Object> globalInstance = context->Global();
  Local<Value> value = message.GetValue();
  if (value.IsEmpty()) value = Undefined(isolate);
  Local<Value> argv[2] = { value, promise };
  Local<Value> func = globalInstance->Get(context, String::NewFromUtf8(isolate, "onUnhandledRejection", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked();
  if (func->IsFunction()) {
    Local<Function> onUnhandledRejection = Local<Function>::Cast(func);
    TryCatch try_catch(isolate);
    onUnhandledRejection->Call(context, globalInstance, 2, argv);
    if (try_catch.HasCaught()) {
      dv8::ReportException(isolate, &try_catch);
    }
  }
}

void CompileScript(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  v8::TryCatch try_catch(isolate);
  v8::Local<v8::String> source = args[0].As<v8::String>();
  v8::Local<v8::String> path = args[1].As<v8::String>();
  v8::Local<v8::Array> params_buf;
  params_buf = args[2].As<v8::Array>();
  v8::Local<v8::Array> context_extensions_buf;
  context_extensions_buf = args[3].As<v8::Array>();
  std::vector<v8::Local<v8::String>> params;
  if (!params_buf.IsEmpty()) {
    for (uint32_t n = 0; n < params_buf->Length(); n++) {
      v8::Local<v8::Value> val;
      if (!params_buf->Get(context, n).ToLocal(&val)) return;
      params.push_back(val.As<v8::String>());
    }
  }
  std::vector<v8::Local<v8::Object>> context_extensions;
  if (!context_extensions_buf.IsEmpty()) {
    for (uint32_t n = 0; n < context_extensions_buf->Length(); n++) {
      v8::Local<v8::Value> val;
      if (!context_extensions_buf->Get(context, n).ToLocal(&val)) return;
      context_extensions.push_back(val.As<v8::Object>());
    }
  }
  v8::ScriptOrigin baseorigin(path, // resource name
    v8::Integer::New(isolate, 0), // line offset
    v8::Integer::New(isolate, 0),  // column offset
    v8::False(isolate), // is shared cross-origin
    v8::Local<v8::Integer>(),  // script id
    v8::Local<v8::Value>(), // source map url
    v8::False(isolate), // is opaque
    v8::False(isolate), // is wasm
    v8::False(isolate)); // is module
  v8::Context::Scope scope(context);
  v8::ScriptCompiler::Source basescript(source, baseorigin);
  v8::Local<v8::ScriptOrModule> script;
  v8::MaybeLocal<v8::Function> maybe_fn = ScriptCompiler::CompileFunctionInContext(
      context, &basescript, params.size(), params.data(),
      context_extensions.size(), context_extensions.data(), v8::ScriptCompiler::kNoCompileOptions,
      v8::ScriptCompiler::NoCacheReason::kNoCacheNoReason, &script);
  if (maybe_fn.IsEmpty()) {
    if (try_catch.HasCaught() && !try_catch.HasTerminated()) {
      try_catch.ReThrow();
    }
    return;
  }
  v8::Local<v8::Function> fn = maybe_fn.ToLocalChecked();
  args.GetReturnValue().Set(fn);
}

void RunModule(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
  v8::TryCatch try_catch(isolate);
  Local<String> source = args[0].As<String>();
  Local<String> path = args[1].As<String>();
  v8::ScriptOrigin baseorigin(path, // resource name
    v8::Integer::New(isolate, 0), // line offset
    v8::Integer::New(isolate, 0),  // column offset
    v8::False(isolate), // is shared cross-origin
    v8::Local<v8::Integer>(),  // script id
    v8::Local<v8::Value>(), // source map url
    v8::False(isolate), // is opaque
    v8::False(isolate), // is wasm
    v8::True(isolate)); // is module
  v8::Local<v8::Module> module;
  v8::ScriptCompiler::Source basescript(source, baseorigin);
  if (!v8::ScriptCompiler::CompileModule(isolate, &basescript).ToLocal(&module)) {
    dv8::PrintStackTrace(isolate, try_catch);
    return;
  }
  v8::Maybe<bool> ok = module->InstantiateModule(context, dv8::OnModuleInstantiate);
  if (!ok.ToChecked()) {
    if (try_catch.HasCaught()) {
      dv8::PrintStackTrace(isolate, try_catch);
    }
    return;
  }
  v8::MaybeLocal<v8::Value> result = module->Evaluate(context);
  if (result.IsEmpty()) {
    if (try_catch.HasCaught()) {
      dv8::PrintStackTrace(isolate, try_catch);
      return;
    }
  }
  args.GetReturnValue().Set(result.ToLocalChecked());
}

void RunScript(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
  v8::TryCatch try_catch(isolate);
  Local<String> source = args[0].As<String>();
  Local<String> path = args[1].As<String>();
  v8::ScriptOrigin baseorigin(path, // resource name
    v8::Integer::New(isolate, 0), // line offset
    v8::Integer::New(isolate, 0),  // column offset
    v8::False(isolate), // is shared cross-origin
    v8::Local<v8::Integer>(),  // script id
    v8::Local<v8::Value>(), // source map url
    v8::False(isolate), // is opaque
    v8::False(isolate), // is wasm
    v8::False(isolate)); // is module
  v8::Local<v8::Script> script;
  v8::ScriptCompiler::Source basescript(source, baseorigin);
  if (!v8::ScriptCompiler::Compile(context, &basescript).ToLocal(&script)) {
    dv8::PrintStackTrace(isolate, try_catch);
    return;
  }
  if (try_catch.HasCaught()) {
    dv8::PrintStackTrace(isolate, try_catch);
    return;
  }
  MaybeLocal<Value> result = script->Run(context);
  if (try_catch.HasCaught()) {
    dv8::PrintStackTrace(isolate, try_catch);
    return;
  }
  args.GetReturnValue().Set(result.ToLocalChecked());
}

void PrintStackTrace(v8::Isolate* isolate, const v8::TryCatch& try_catch) {
  v8::Local<v8::Value> exception = try_catch.Exception();
  v8::Local<v8::Message> message = try_catch.Message();
  v8::Local<v8::StackTrace> stack = message->GetStackTrace();
  v8::String::Utf8Value ex(isolate, exception);
  v8::Local<v8::Value> scriptName = message->GetScriptResourceName();
  v8::String::Utf8Value scriptname(isolate, scriptName);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  int linenum = message->GetLineNumber(context).FromJust();
  fprintf(stderr, "%s in %s on line %i\n", *ex, *scriptname, linenum);
  //fprintf(stderr, "frames: %i\n", stack->GetFrameCount());
  for (int i = 0; i < stack->GetFrameCount(); i++) {
    v8::Local<v8::StackFrame> stack_frame = stack->GetFrame(isolate, i);
    v8::Local<v8::String> functionName = stack_frame->GetFunctionName();
    v8::Local<v8::String> scriptName = stack_frame->GetScriptName();
    
    v8::String::Utf8Value fn_name_s(isolate, functionName);
    v8::String::Utf8Value script_name(isolate, scriptName);

    const int line_number = stack_frame->GetLineNumber();
    const int column = stack_frame->GetColumn();
    if (stack_frame->IsEval()) {
      if (stack_frame->GetScriptId() == v8::Message::kNoScriptIdInfo) {
        fprintf(stderr, "    at [eval]:%i:%i\n", line_number, column);
      } else {
        fprintf(stderr,
                "    at [eval] (%s:%i:%i)\n",
                *script_name,
                line_number,
                column);
      }
      break;
    }
    if (fn_name_s.length() == 0) {
      fprintf(stderr, "    at %s:%i:%i\n", *script_name, line_number, column);
    } else {
      fprintf(stderr,
              "    at %s (%s:%i:%i)\n",
              *fn_name_s,
              *script_name,
              line_number,
              column);
    }
  }
  fflush(stderr);
}

void ReportException(Isolate *isolate, TryCatch *try_catch) {
  HandleScope handle_scope(isolate);
  Local<Context> context(isolate->GetCurrentContext());
  Local<Object> globalInstance = context->Global();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
  Local<Value> er = try_catch->Exception();
  Local<Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    message = Exception::CreateMessage(isolate, er);
  }
  Local<Value> func = globalInstance->Get(context, String::NewFromUtf8(isolate, "onUncaughtException", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked();
  if (func->IsFunction()) {
    Local<Function> onUncaughtException = Local<Function>::Cast(func);
    Local<Object> err_obj = er->ToObject(context).ToLocalChecked();
    Local<Value> stack_trace_string;
    if (try_catch->StackTrace(context).ToLocal(&stack_trace_string)) {
      String::Utf8Value stack_trace(isolate, stack_trace_string);
      char *stack_trace_string = *stack_trace;
      err_obj->Set(context, String::NewFromUtf8(isolate, "stack", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, stack_trace_string, v8::NewStringType::kNormal).ToLocalChecked());
    }
    Local<Value> argv[1] = { err_obj };
    onUncaughtException->Call(context, globalInstance, 1, argv);
  } else {
    dv8::PrintStackTrace(isolate, *try_catch);
  }
}

void Print(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  String::Utf8Value str(args.GetIsolate(), args[0]);
  int endline = 1;
  if (args.Length() > 1) {
    endline = static_cast<int>(args[1]->BooleanValue(isolate));
  }
  const char *cstr = *str;
  if (endline == 1) {
    fprintf(stdout, "%s\n", cstr);
  } else {
    fprintf(stdout, "%s", cstr);
  }
}

void Err(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  String::Utf8Value str(args.GetIsolate(), args[0]);
  int endline = 1;
  if (args.Length() > 1) {
    endline = static_cast<int>(args[1]->BooleanValue(isolate));
  }
  const char *cstr = *str;
  if (endline == 1) {
    fprintf(stderr, "%s\n", cstr);
  } else {
    fprintf(stderr, "%s", cstr);
  }
  fflush(stderr);
}

MaybeLocal<Module> OnModuleInstantiate(Local<Context> context, Local<String> specifier, Local<Module> referrer) {
  HandleScope handle_scope(context->GetIsolate());
  return MaybeLocal<Module>();
}

void EnvVars(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  int size = 0;
  while (environ[size]) size++;
  Local<v8::Array> envarr = v8::Array::New(isolate);
  for (int i = 0; i < size; ++i) {
    const char *var = environ[i];
    envarr->Set(context, i, String::NewFromUtf8(isolate, var, v8::NewStringType::kNormal, strlen(var)).ToLocalChecked());
  }
  args.GetReturnValue().Set(envarr);
}

inline ssize_t process_memory_usage() {
  char buf[1024];
  const char* s = NULL;
  ssize_t n = 0;
  long val = 0;
  int fd = 0;
  int i = 0;
  do {
    fd = open("/proc/self/stat", O_RDONLY);
  } while (fd == -1 && errno == EINTR);
  if (fd == -1) return (ssize_t)errno;
  do
    n = read(fd, buf, sizeof(buf) - 1);
  while (n == -1 && errno == EINTR);
  close(fd);
  if (n == -1)
    return (ssize_t)errno;
  buf[n] = '\0';
  s = strchr(buf, ' ');
  if (s == NULL)
    goto err;
  s += 1;
  if (*s != '(')
    goto err;
  s = strchr(s, ')');
  if (s == NULL)
    goto err;
  for (i = 1; i <= 22; i++) {
    s = strchr(s + 1, ' ');
    if (s == NULL)
      goto err;
  }
  errno = 0;
  val = strtol(s, NULL, 10);
  if (errno != 0)
    goto err;
  if (val < 0)
    goto err;
  return val * getpagesize();
err:
  return 0;
}

void MemoryUsage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  ssize_t rss = process_memory_usage();
  HeapStatistics v8_heap_stats;
  isolate->GetHeapStatistics(&v8_heap_stats);
  Local<Float64Array> array = args[0].As<Float64Array>();
  Local<ArrayBuffer> ab = array->Buffer();
  double *fields = static_cast<double *>(ab->GetContents().Data());
  fields[0] = rss;
  fields[1] = v8_heap_stats.total_heap_size();
  fields[2] = v8_heap_stats.used_heap_size();
  fields[3] = v8_heap_stats.external_memory();
  fields[4] = v8_heap_stats.does_zap_garbage();
  fields[5] = v8_heap_stats.heap_size_limit();
  fields[6] = v8_heap_stats.malloced_memory();
  fields[7] = v8_heap_stats.number_of_detached_contexts();
  fields[8] = v8_heap_stats.number_of_native_contexts();
  fields[9] = v8_heap_stats.peak_malloced_memory();
  fields[10] = v8_heap_stats.total_available_size();
  fields[11] = v8_heap_stats.total_heap_size_executable();
  fields[12] = v8_heap_stats.total_physical_size();
  fields[13] = isolate->AdjustAmountOfExternalAllocatedMemory(0);
  args.GetReturnValue().Set(array);
}

void HeapSpaceUsage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  HeapSpaceStatistics s;
  size_t number_of_heap_spaces = isolate->NumberOfHeapSpaces();
  Local<Array> spaces = args[0].As<Array>();
  Local<Object> o = Object::New(isolate);
  HeapStatistics v8_heap_stats;
  isolate->GetHeapStatistics(&v8_heap_stats);
  Local<Object> heaps = Object::New(isolate);
  o->Set(context, String::NewFromUtf8(isolate, "totalMemory", v8::NewStringType::kNormal).ToLocalChecked(), Integer::New(isolate, v8_heap_stats.total_heap_size()));
  o->Set(context, String::NewFromUtf8(isolate, "totalCommittedMemory", v8::NewStringType::kNormal).ToLocalChecked(), Integer::New(isolate, v8_heap_stats.total_physical_size()));
  o->Set(context, String::NewFromUtf8(isolate, "usedMemory", v8::NewStringType::kNormal).ToLocalChecked(), Integer::New(isolate, v8_heap_stats.used_heap_size()));
  o->Set(context, String::NewFromUtf8(isolate, "availableMemory", v8::NewStringType::kNormal).ToLocalChecked(), Integer::New(isolate, v8_heap_stats.total_available_size()));
  o->Set(context, String::NewFromUtf8(isolate, "memoryLimit", v8::NewStringType::kNormal).ToLocalChecked(), Integer::New(isolate, v8_heap_stats.heap_size_limit()));
  o->Set(context, String::NewFromUtf8(isolate, "heapSpaces", v8::NewStringType::kNormal).ToLocalChecked(), heaps);
  for (size_t i = 0; i < number_of_heap_spaces; i++) {
    isolate->GetHeapSpaceStatistics(&s, i);
    Local<Float64Array> array = spaces->Get(context, i).ToLocalChecked().As<Float64Array>();
    Local<ArrayBuffer> ab = array->Buffer();
    double *fields = static_cast<double *>(ab->GetContents().Data());
    fields[0] = s.physical_space_size();
    fields[1] = s.space_available_size();
    fields[2] = s.space_size();
    fields[3] = s.space_used_size();
    heaps->Set(context, String::NewFromUtf8(isolate, s.space_name(), v8::NewStringType::kNormal).ToLocalChecked(), array);
  }
  args.GetReturnValue().Set(o);
}


void PID(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  args.GetReturnValue().Set(Integer::New(isolate, getpid()));
}

void CPUUsage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  struct rusage usage;
  jsys_usage(&usage);
  Local<Float64Array> array = args[0].As<Float64Array>();
  Local<ArrayBuffer> ab = array->Buffer();
  double *fields = static_cast<double *>(ab->GetContents().Data());
  fields[0] = (MICROS_PER_SEC * usage.ru_utime.tv_sec) + usage.ru_utime.tv_usec;
  fields[1] = (MICROS_PER_SEC * usage.ru_stime.tv_sec) + usage.ru_stime.tv_usec;
}

inline uint64_t hrtime() {
  static clock_t fast_clock_id = -1;
  struct timespec t;
  clock_t clock_id = CLOCK_MONOTONIC;
  if (clock_gettime(clock_id, &t))
    return 0;
  return t.tv_sec * (uint64_t) 1e9 + t.tv_nsec;
}

void HRTime(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<ArrayBuffer> ab = args[0].As<BigUint64Array>()->Buffer();
  uint64_t *fields = static_cast<uint64_t *>(ab->GetContents().Data());
  fields[0] = hrtime();
}

void RunMicroTasks(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::MicrotasksScope::PerformCheckpoint(isolate);
  //isolate->RunMicrotasks();
}

void EnqueueMicrotask(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  isolate->EnqueueMicrotask(args[0].As<Function>());
}

void Exit(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  int status = args[0]->Int32Value(context).ToChecked();
  exit(status);
}

void Cwd(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  char cwd[PATH_MAX];
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, getcwd(cwd, PATH_MAX), NewStringType::kNormal).ToLocalChecked());
}

void USleep(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  int microseconds = args[0]->IntegerValue(context).ToChecked();
  usleep(microseconds);
}

// TODO: could autogenerate which of these are available on dv8 object in config
Local<Context> CreateContext(Isolate *isolate) {
  Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
  Local<ObjectTemplate> dv8 = ObjectTemplate::New(isolate);
  dv8->Set(String::NewFromUtf8(isolate, "print", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Print));
  dv8->Set(String::NewFromUtf8(isolate, "error", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Err));
  dv8->Set(String::NewFromUtf8(isolate, "memoryUsage", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, MemoryUsage));
  dv8->Set(String::NewFromUtf8(isolate, "library", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, LoadModule));
  dv8->Set(String::NewFromUtf8(isolate, "env", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, EnvVars));
  dv8->Set(String::NewFromUtf8(isolate, "runScript", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, RunScript));
  dv8->Set(String::NewFromUtf8(isolate, "runModule", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, RunModule));
  dv8->Set(String::NewFromUtf8(isolate, "compile", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, CompileScript));
  dv8->Set(String::NewFromUtf8(isolate, "pid", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, PID));
  dv8->Set(String::NewFromUtf8(isolate, "cpuUsage", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, CPUUsage));
  dv8->Set(String::NewFromUtf8(isolate, "hrtime", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, HRTime));
  dv8->Set(String::NewFromUtf8(isolate, "heapUsage", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, HeapSpaceUsage));
  dv8->Set(String::NewFromUtf8(isolate, "cwd", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Cwd));
  dv8->Set(String::NewFromUtf8(isolate, "usleep", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, USleep));
  dv8->Set(String::NewFromUtf8(isolate, "exit", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Exit));
  dv8->Set(String::NewFromUtf8(isolate, "runMicroTasks", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, RunMicroTasks));
  dv8->Set(String::NewFromUtf8(isolate, "enqueueMicroTask", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, EnqueueMicrotask));
  dv8->Set(String::NewFromUtf8(isolate, "v8", NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, v8::V8::GetVersion()).ToLocalChecked());
  dv8->Set(String::NewFromUtf8(isolate, "version", NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, "0.1.0").ToLocalChecked());
  dv8->Set(String::NewFromUtf8(isolate, "glibc", NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, gnu_get_libc_version()).ToLocalChecked());
  Local<ObjectTemplate> kernel = ObjectTemplate::New(isolate);
  utsname kernel_rec;
  int rc = uname(&kernel_rec);
  if (rc == 0) {
    kernel->Set(String::NewFromUtf8(isolate, "os", NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, kernel_rec.sysname).ToLocalChecked());
    kernel->Set(String::NewFromUtf8(isolate, "release", NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, kernel_rec.release).ToLocalChecked());
    kernel->Set(String::NewFromUtf8(isolate, "version", NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, kernel_rec.version).ToLocalChecked());
  }
  dv8->Set(String::NewFromUtf8(isolate, "kernel", NewStringType::kNormal).ToLocalChecked(), kernel);
  global->Set(String::NewFromUtf8(isolate, "dv8", NewStringType::kNormal).ToLocalChecked(), dv8);
  Local<Context> context = Context::New(isolate, NULL, global);
  context->AllowCodeGenerationFromStrings(false);
  return context;
}

} // namespace dv8
