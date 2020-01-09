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
  fprintf(stderr, "frames: %i\n", stack->GetFrameCount());
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
  /*
    String::Utf8Value filename(isolate, message->GetScriptResourceName());
    env->err.Reset(isolate, err_obj);
    env->error->hasError = 1;
    String::Utf8Value exception(isolate, er);
    char *exception_string = *exception;
    char *filename_string = *filename;
    int linenum = message->GetLineNumber(context).FromJust();
    env->error->linenum = linenum;
    env->error->filename = (char*)calloc(strlen(filename_string), 1);
    memcpy(env->error->filename, filename_string, strlen(filename_string));
    env->error->exception = (char*)calloc(strlen(exception_string), 1);
    memcpy(env->error->exception, exception_string, strlen(exception_string));
    String::Utf8Value sourceline(isolate, message->GetSourceLine(context).ToLocalChecked());
    char *sourceline_string = *sourceline;
    env->error->sourceline = (char*)calloc(strlen(sourceline_string), 1);
    memcpy(env->error->sourceline, sourceline_string, strlen(sourceline_string));
    Local<v8::StackTrace> trace = message->GetStackTrace();
    int frame_count = trace->GetFrameCount();
    v8::Local<v8::Array> stack = v8::Array::New(isolate);

    err_obj->Set(context, String::NewFromUtf8(isolate, "fileName", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, filename_string, v8::NewStringType::kNormal).ToLocalChecked());
    err_obj->Set(context, String::NewFromUtf8(isolate, "lineNumber", v8::NewStringType::kNormal).ToLocalChecked(), Integer::New(isolate, linenum));
    //err_obj->Set(context, String::NewFromUtf8(isolate, "exception", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, exception_string, v8::NewStringType::kNormal).ToLocalChecked());
    //err_obj->Set(context, String::NewFromUtf8(isolate, "sourceLine", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, sourceline_string, v8::NewStringType::kNormal).ToLocalChecked());
    //err_obj->Set(context, String::NewFromUtf8(isolate, "frames", v8::NewStringType::kNormal).ToLocalChecked(), Integer::New(isolate, frame_count));
    for (int i = 0; i < frame_count; i++) {
      Local<Object> frame = Object::New(isolate);
      frame->Set(context, String::NewFromUtf8(isolate, "line", v8::NewStringType::kNormal).ToLocalChecked(), Integer::New(isolate, trace->GetFrame(isolate, i)->GetLineNumber()));
      frame->Set(context, String::NewFromUtf8(isolate, "column", v8::NewStringType::kNormal).ToLocalChecked(), Integer::New(isolate, trace->GetFrame(isolate, i)->GetColumn()));
      //frame->Set(context, String::NewFromUtf8(isolate, "scriptId", v8::NewStringType::kNormal).ToLocalChecked(), Integer::New(isolate, trace->GetFrame(isolate, i)->GetScriptId()));
      frame->Set(context, String::NewFromUtf8(isolate, "isEval", v8::NewStringType::kNormal).ToLocalChecked(), v8::Boolean::New(isolate, trace->GetFrame(isolate, i)->IsEval()));
      frame->Set(context, String::NewFromUtf8(isolate, "isConstructor", v8::NewStringType::kNormal).ToLocalChecked(), v8::Boolean::New(isolate, trace->GetFrame(isolate, i)->IsConstructor()));
      frame->Set(context, String::NewFromUtf8(isolate, "isWasm", v8::NewStringType::kNormal).ToLocalChecked(), v8::Boolean::New(isolate, trace->GetFrame(isolate, i)->IsWasm()));
      frame->Set(context, String::NewFromUtf8(isolate, "isUserJavascript", v8::NewStringType::kNormal).ToLocalChecked(), v8::Boolean::New(isolate, trace->GetFrame(isolate, i)->IsUserJavaScript()));
      Local<String> functionName = trace->GetFrame(isolate, i)->GetFunctionName();
      if (!functionName.IsEmpty()) {
        frame->Set(context, String::NewFromUtf8(isolate, "functionName", v8::NewStringType::kNormal).ToLocalChecked(), functionName);
      }
      Local<String> scriptName = trace->GetFrame(isolate, i)->GetScriptName();
      if (!scriptName.IsEmpty()) {
        frame->Set(context, String::NewFromUtf8(isolate, "scriptName", v8::NewStringType::kNormal).ToLocalChecked(), scriptName);
      }
      Local<String> scriptNameOrSourceUrl = trace->GetFrame(isolate, i)->GetScriptNameOrSourceURL();
      if (!scriptNameOrSourceUrl.IsEmpty()) {
        frame->Set(context, String::NewFromUtf8(isolate, "scriptNameOrSourceUrl", v8::NewStringType::kNormal).ToLocalChecked(), scriptNameOrSourceUrl);
      }
      stack->Set(context, i, frame);
    }
    err_obj->Set(context, String::NewFromUtf8(isolate, "stack", v8::NewStringType::kNormal).ToLocalChecked(), stack);
  */
    Local<Value> stack_trace_string;
    if (try_catch->StackTrace(context).ToLocal(&stack_trace_string)) {
      String::Utf8Value stack_trace(isolate, stack_trace_string);
      char *stack_trace_string = *stack_trace;
      env->error->stack = (char*)calloc(strlen(stack_trace_string), 1);
      err_obj->Set(context, String::NewFromUtf8(isolate, "stack", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, stack_trace_string, v8::NewStringType::kNormal).ToLocalChecked());
      memcpy(env->error->stack, stack_trace_string, strlen(stack_trace_string));
    }
    Local<Value> argv[1] = { err_obj };
    onUncaughtException->Call(context, globalInstance, 1, argv);
  }
}

void Version(const FunctionCallbackInfo<Value> &args) {
  args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), v8::V8::GetVersion(), NewStringType::kNormal).ToLocalChecked());
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
    fprintf(stderr, "%s\n", cstr);
  } else {
    fprintf(stderr, "%s", cstr);
  }
  fflush(stderr);
}

void LoadModule(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  String::Utf8Value str(args.GetIsolate(), args[0]);
  const char *module_name = *str;
  Local<Object> exports;
  bool ok = args[1]->ToObject(context).ToLocal(&exports);
  if (!ok) {
    args.GetReturnValue().Set(Null(isolate));
    return;
  }
  args.GetReturnValue().Set(exports);
#if V8_DLOPEN
  const char *module_path = "/usr/local/lib/dv8/";
  char lib_name[1024];
  if (args.Length() > 2) {
    String::Utf8Value str(args.GetIsolate(), args[2]);
    module_path = *str;
    snprintf(lib_name, 1024, "%s%s.so", module_path, module_name);
  } else {
    if (strcmp("loop", module_name) == 0) {
      dv8::loop::InitAll(exports);
      return;
    } else if (strcmp("socket", module_name) == 0) {
      dv8::socket::InitAll(exports);
      return;
    } else if (strcmp("timer", module_name) == 0) {
      dv8::timer::InitAll(exports);
      return;
    } else if (strcmp("zlib", module_name) == 0) {
      dv8::libz::InitAll(exports);
      return;
    } else if (strcmp("thread", module_name) == 0) {
      dv8::thread::InitAll(exports);
      return;
    } else if (strcmp("fs", module_name) == 0) {
      dv8::fs::InitAll(exports);
      return;
    } else if (strcmp("udp", module_name) == 0) {
      dv8::udp::InitAll(exports);
      return;
    } else if (strcmp("process", module_name) == 0) {
      dv8::process::InitAll(exports);
      return;
    } else if (strcmp("tty", module_name) == 0) {
      dv8::tty::InitAll(exports);
      return;
    } else if (strcmp("openssl", module_name) == 0) {
      dv8::openssl::InitAll(exports);
      return;
    } else if (strcmp("os", module_name) == 0) {
      dv8::os::InitAll(exports);
      return;
    }
    snprintf(lib_name, 1024, "%s%s.so", module_path, module_name);
  }
  uv_lib_t lib;
  args.GetReturnValue().Set(exports);
  fprintf(stderr, "%s\n", lib_name);
  int success = uv_dlopen(lib_name, &lib);
  if (success != 0) {
    isolate->ThrowException(v8::Exception::Error(String::NewFromUtf8(isolate, "uv_dlopen failed").ToLocalChecked()));
    return;
  }
  char register_name[128];
  snprintf(register_name, 128, "_register_%s", module_name);
  void *address;
  success = uv_dlsym(&lib, register_name, &address);
  if (success != 0) {
    isolate->ThrowException(v8::Exception::Error(String::NewFromUtf8(isolate, "uv_dlsym failed").ToLocalChecked()));
    return;
  }
  register_plugin _init = reinterpret_cast<register_plugin>(address);
  auto _register = reinterpret_cast<InitializerCallback>(_init());
  _register(exports);
#else
  if (strcmp("loop", module_name) == 0) {
    dv8::loop::InitAll(exports);
    return;
  } else if (strcmp("socket", module_name) == 0) {
    dv8::socket::InitAll(exports);
    return;
  } else if (strcmp("timer", module_name) == 0) {
    dv8::timer::InitAll(exports);
    return;
  } else if (strcmp("zlib", module_name) == 0) {
    dv8::libz::InitAll(exports);
    return;
  } else if (strcmp("thread", module_name) == 0) {
    dv8::thread::InitAll(exports);
    return;
  } else if (strcmp("fs", module_name) == 0) {
    dv8::fs::InitAll(exports);
    return;
  } else if (strcmp("udp", module_name) == 0) {
    dv8::udp::InitAll(exports);
    return;
  } else if (strcmp("process", module_name) == 0) {
    dv8::process::InitAll(exports);
    return;
  } else if (strcmp("tty", module_name) == 0) {
    dv8::tty::InitAll(exports);
    return;
  } else if (strcmp("openssl", module_name) == 0) {
    dv8::openssl::InitAll(exports);
    return;
  } else if (strcmp("os", module_name) == 0) {
    dv8::os::InitAll(exports);
    return;
  }
  isolate->ThrowException(v8::Exception::Error(String::NewFromUtf8(isolate, "Module Not Found").ToLocalChecked()));
#endif
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

void MemoryUsage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  size_t rss;
  int err = uv_resident_set_memory(&rss);
  if (err) {
    return args.GetReturnValue().Set(String::NewFromUtf8(isolate, uv_strerror(err), v8::NewStringType::kNormal).ToLocalChecked());
  }
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

Local<Context> CreateContext(Isolate *isolate) {
  Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
  Local<ObjectTemplate> dv8 = ObjectTemplate::New(isolate);
  dv8->Set(String::NewFromUtf8(isolate, "version", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Version));
  dv8->Set(String::NewFromUtf8(isolate, "print", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Print));
  dv8->Set(String::NewFromUtf8(isolate, "memoryUsage", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, MemoryUsage));
  dv8->Set(String::NewFromUtf8(isolate, "library", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, LoadModule));
  dv8->Set(String::NewFromUtf8(isolate, "env", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, EnvVars));
  dv8->Set(String::NewFromUtf8(isolate, "runScript", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, RunScript));
  dv8->Set(String::NewFromUtf8(isolate, "compile", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, CompileScript));
  global->Set(String::NewFromUtf8(isolate, "dv8", NewStringType::kNormal).ToLocalChecked(), dv8);
  Local<Context> context = Context::New(isolate, NULL, global);
  context->AllowCodeGenerationFromStrings(false);
  return context;
}

} // namespace dv8
