#include <dv8.h>

namespace dv8 {

void PromiseRejectCallback(PromiseRejectMessage message) {
  Local<Promise> promise = message.GetPromise();
  Isolate* isolate = promise->GetIsolate();
  HandleScope handle_scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  PromiseRejectEvent event = message.GetEvent();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(32));
  if (!env->onUnhandledRejection.IsEmpty() && event == v8::kPromiseRejectWithNoHandler) {
    const unsigned int argc = 3;
    Local<Object> globalInstance = context->Global();
    Local<Value> value = message.GetValue();
    if (value.IsEmpty()) value = Undefined(isolate);
    Local<Value> argv[argc] = { promise, value, Integer::New(isolate, event) };
    Local<Function> onUnhandledRejection = Local<Function>::New(isolate, env->onUnhandledRejection);
    TryCatch try_catch(isolate);
    onUnhandledRejection->Call(globalInstance, 3, argv);
    if (try_catch.HasCaught()) {
      dv8::ReportException(isolate, &try_catch);
    }
  }
}

void on_handle_close(uv_handle_t *h) {
  free(h);
}

void shutdown(uv_loop_t *loop) {
  uv_walk(loop, [](uv_handle_t *handle, void *arg) {
    const char* typeName = uv_handle_type_name(handle->type);
    //fprintf(stderr, "closing [%p] %s in state: %i\n", handle, uv_handle_type_name(handle->type), uv_is_active(handle));
    uv_close(handle, on_handle_close);
  }, NULL);
}

void Shutdown(const FunctionCallbackInfo<Value> &args) {
  shutdown(uv_default_loop());
}

void ReportException(Isolate *isolate, TryCatch *try_catch) {
  HandleScope handle_scope(isolate);
  Local<Context> context(isolate->GetCurrentContext());
  Local<Object> globalInstance = context->Global();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(32));
  Local<Value> er = try_catch->Exception();
  Local<Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    message = Exception::CreateMessage(isolate, er);
  }
  String::Utf8Value filename(isolate, message->GetScriptOrigin().ResourceName());
  Local<Value> func = globalInstance->Get(String::NewFromUtf8(isolate, "onUncaughtException", NewStringType::kNormal).ToLocalChecked());
  Local<Function> onUncaughtException = Local<Function>::Cast(func);
  Local<Object> err_obj = er->ToObject(context).ToLocalChecked();
  env->err.Reset(isolate, err_obj);
  env->error->hasError = 1;
  String::Utf8Value exception(isolate, er);
  char *exception_string = *exception;
  char *filename_string = *filename;
  int linenum = message->GetLineNumber(context).FromJust();
  env->error->linenum = linenum;
  env->error->filename = (char*)calloc(strlen(filename_string), 1);
  memcpy(env->error->filename, filename_string, strlen(filename_string));
  err_obj->Set(String::NewFromUtf8(isolate, "fileName", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, filename_string, v8::NewStringType::kNormal).ToLocalChecked());
  env->error->exception = (char*)calloc(strlen(exception_string), 1);
  memcpy(env->error->exception, exception_string, strlen(exception_string));
  err_obj->Set(String::NewFromUtf8(isolate, "exception", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, exception_string, v8::NewStringType::kNormal).ToLocalChecked());
  String::Utf8Value sourceline(isolate, message->GetSourceLine(context).ToLocalChecked());
  char *sourceline_string = *sourceline;
  env->error->sourceline = (char*)calloc(strlen(sourceline_string), 1);
  memcpy(env->error->sourceline, sourceline_string, strlen(sourceline_string));
  err_obj->Set(String::NewFromUtf8(isolate, "sourceLine", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, sourceline_string, v8::NewStringType::kNormal).ToLocalChecked());
  Local<Value> stack_trace_string;
  if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) && stack_trace_string->IsString() && Local<String>::Cast(stack_trace_string)->Length() > 0) {
    String::Utf8Value stack_trace(isolate, stack_trace_string);
    char *stack_trace_string = *stack_trace;
    env->error->stack = (char*)calloc(strlen(stack_trace_string), 1);
    err_obj->Set(String::NewFromUtf8(isolate, "stack", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, stack_trace_string, v8::NewStringType::kNormal).ToLocalChecked());
    memcpy(env->error->stack, stack_trace_string, strlen(stack_trace_string));
  }
  Local<Value> argv[1] = { err_obj };
  onUncaughtException->Call(globalInstance, 1, argv);
}

void Version(const FunctionCallbackInfo<Value> &args) {
  args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), v8::V8::GetVersion(), NewStringType::kNormal).ToLocalChecked());
}

void Print(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  bool first = true;
  int len = args.Length();
  for (int i = 0; i < len; i++) {
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    String::Utf8Value str(args.GetIsolate(), args[i]);
    const char *cstr = *str;
    fprintf(stderr, "%s\n", cstr);
    fflush(stderr);
  }
}

void LoadModule(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  String::Utf8Value str(args.GetIsolate(), args[0]);
  static std::atomic<uint64_t> inits{0};
  const char *module_name = *str;
  char lib_name[128];
  Local<Object> exports;
  bool ok = args[1]->ToObject(context).ToLocal(&exports);
  if (!ok) {
    args.GetReturnValue().Set(Null(isolate));
    return;
  }
#ifdef STATIC_BUILD
  if (strcmp("loop", module_name) == 0) {
		dv8::loop::EventLoop::Init(exports);
    args.GetReturnValue().Set(exports);
    return;
  } else if (strcmp("socket", module_name) == 0) {
		dv8::socket::Socket::Init(exports);
    args.GetReturnValue().Set(exports);
    return;
  } else if (strcmp("timer", module_name) == 0) {
		dv8::timer::Timer::Init(exports);
    args.GetReturnValue().Set(exports);
    return;
  } else if (strcmp("thread", module_name) == 0) {
		dv8::thread::Thread::Init(exports);
    args.GetReturnValue().Set(exports);
    return;
  } else if (strcmp("process", module_name) == 0) {
		dv8::process::Process::Init(exports);
    args.GetReturnValue().Set(exports);
    return;
  } else if (strcmp("udp", module_name) == 0) {
		dv8::udp::UDP::Init(exports);
    args.GetReturnValue().Set(exports);
    return;
  } else if (strcmp("os", module_name) == 0) {
		dv8::os::OS::Init(exports);
    args.GetReturnValue().Set(exports);
    return;
  } else if (strcmp("tty", module_name) == 0) {
		dv8::tty::TTY::Init(exports);
    args.GetReturnValue().Set(exports);
    return;
  } else if (strcmp("os", module_name) == 0) {
		dv8::os::OS::Init(exports);
    args.GetReturnValue().Set(exports);
    return;
  } else if (strcmp("fs", module_name) == 0) {
		dv8::fs::FileSystem::Init(exports);
		dv8::fs::File::Init(exports);
    args.GetReturnValue().Set(exports);
    return;
  } else if (strcmp("libz", module_name) == 0) {
		dv8::libz::ZLib::Init(exports);
    args.GetReturnValue().Set(exports);
    return;
  } else if (strcmp("openssl", module_name) == 0) {
		inits++;
		int loads = inits.load();
		dv8::openssl::Hash::Init(exports);
		dv8::openssl::Hmac::Init(exports);
		dv8::openssl::SecureContext::Init(exports);
		dv8::openssl::SecureSocket::Init(exports);
		if (loads == 1) {
			SSL_library_init();
			BIO* bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);
			SSL_load_error_strings();
			ERR_load_BIO_strings();
			OpenSSL_add_all_algorithms();
			ERR_load_crypto_strings();
		}
    args.GetReturnValue().Set(exports);
    return;
  } else if (strcmp("httpParser", module_name) == 0) {
		dv8::httpParser::HTTPParser::Init(exports);
    args.GetReturnValue().Set(exports);
    return;
  }
#endif
  snprintf(lib_name, 128, "/usr/local/lib/%s.so", module_name);
  uv_lib_t lib;
  int success = uv_dlopen(lib_name, &lib);
  if (success != 0) {
    fprintf(stderr, "uv_dlopen failed: %i\n", success);
    fprintf(stderr, "%s\n", uv_dlerror(&lib));
    args.GetReturnValue().Set(Null(isolate));
    return;
  }
  char register_name[128];
  snprintf(register_name, 128, "_register_%s", module_name);
  void *address;
  success = uv_dlsym(&lib, register_name, &address);
  if (success != 0) {
    fprintf(stderr, "uv_dlsym failed: %i\n", success);
    args.GetReturnValue().Set(Null(isolate));
    return;
  }
  register_plugin _init = reinterpret_cast<register_plugin>(address);
  auto _register = reinterpret_cast<InitializerCallback>(_init());
  _register(exports);
  args.GetReturnValue().Set(exports);
  uv_dlclose(&lib);
}

MaybeLocal<Module> OnModuleInstantiate(Local<Context> context, Local<String> specifier, Local<Module> referrer) {
  HandleScope handle_scope(context->GetIsolate());
  return MaybeLocal<Module>();
}

void CollectGarbage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  isolate->RequestGarbageCollectionForTesting(v8::Isolate::kFullGarbageCollection);
}

void EnvVars(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  int size = 0;
  while (environ[size]) size++;
  Local<v8::Array> envarr = v8::Array::New(isolate);
  for (int i = 0; i < size; ++i) {
    const char *var = environ[i];
    envarr->Set(i, String::NewFromUtf8(isolate, var, v8::String::kNormalString, strlen(var)));
  }
  args.GetReturnValue().Set(envarr);
}

void OnExit(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(32));
  if (args[0]->IsFunction()) {
    Local<Function> onExit = Local<Function>::Cast(args[0]);
    env->onExit.Reset(isolate, onExit);
  }
}

void MemoryUsage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  size_t rss;
  int err = uv_resident_set_memory(&rss);
  if (err) {
    return args.GetReturnValue().Set(String::NewFromUtf8(isolate, uv_strerror(err), v8::String::kNormalString));
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
}

void OnUnhandledRejection(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(32));
  if (args[0]->IsFunction()) {
    Local<Function> onUnhandledRejection = Local<Function>::Cast(args[0]);
    env->onUnhandledRejection.Reset(isolate, onUnhandledRejection);
  }
}

Local<Context> CreateContext(Isolate *isolate) {
  Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
  global->Set(String::NewFromUtf8(isolate, "version", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Version));
  global->Set(String::NewFromUtf8(isolate, "print", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Print));
  global->Set(String::NewFromUtf8(isolate, "memoryUsage", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, MemoryUsage));
  global->Set(String::NewFromUtf8(isolate, "library", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, LoadModule));
  global->Set(String::NewFromUtf8(isolate, "shutdown", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Shutdown));
  global->Set(String::NewFromUtf8(isolate, "gc", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, CollectGarbage));
  global->Set(String::NewFromUtf8(isolate, "env", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, EnvVars));
  global->Set(String::NewFromUtf8(isolate, "onExit", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, OnExit));
  global->Set(String::NewFromUtf8(isolate, "onUnhandledRejection", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, OnUnhandledRejection));
  return Context::New(isolate, NULL, global);
}

} // namespace dv8
