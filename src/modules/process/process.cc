#include <process.h>

namespace dv8 {

namespace process {
using dv8::builtins::Environment;
using dv8::socket::Socket;

void InitAll(Local<Object> exports)
{
  Process::Init(exports);
}

void Process::Init(Local<Object> exports) {
  Isolate *isolate = exports->GetIsolate();
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Process").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "pid", Process::PID);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "memoryUsage", Process::MemoryUsage);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "heapUsage", Process::HeapSpaceUsage);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "cpuUsage", Process::CPUUsage);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "hrtime", Process::HRTime);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "sleep", Process::Sleep);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "cwd", Process::Cwd);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "usleep", Process::USleep);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "nanosleep", Process::NanoSleep);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "runMicroTasks", Process::RunMicroTasks);
  // spawn, kill, getTitle, setTitle, rss, uptime, rusage, ppid, interfaces, loadavg, exepath, cwd, chdir, homedir, tmpdir, passwd, memory, handles, hostname, getPriority, setPriority 
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "spawn", Process::Spawn);
  DV8_SET_EXPORT(isolate, tpl, "Process", exports);
}

void Process::New(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handle_scope(isolate);
  if (args.IsConstructCall()) {
    Process *obj = new Process();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  }
}

void Process::PID(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  args.GetReturnValue().Set(Integer::New(isolate, uv_os_getpid()));
}

void Process::HeapSpaceUsage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
  HeapSpaceStatistics s;
  size_t number_of_heap_spaces = isolate->NumberOfHeapSpaces();
  Local<Object> o = Object::New(isolate);
  size_t argc = args.Length();
  if (argc < number_of_heap_spaces) {
    number_of_heap_spaces = argc;
  }
  for (size_t i = 0; i < number_of_heap_spaces; i++) {
    isolate->GetHeapSpaceStatistics(&s, i);
    Local<Float64Array> array = args[i].As<Float64Array>();
    Local<ArrayBuffer> ab = array->Buffer();
    double *fields = static_cast<double *>(ab->GetContents().Data());
    fields[0] = s.physical_space_size();
    fields[1] = s.space_available_size();
    fields[2] = s.space_size();
    fields[3] = s.space_used_size();
    o->Set(context, String::NewFromUtf8(isolate, s.space_name(), v8::NewStringType::kNormal).ToLocalChecked(), array);
  }
  args.GetReturnValue().Set(o);
}

void Process::RunMicroTasks(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  isolate->RunMicrotasks();
}

void Process::Sleep(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  int seconds = args[0]->IntegerValue(context).ToChecked();
  sleep(seconds);
}

void Process::Cwd(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  char* cwd = (char*)calloc(1, PATH_MAX);
  size_t size;
  int r = uv_cwd(cwd, &size);
  if (r == UV_ENOBUFS) {
    free(cwd);
    cwd = (char*)calloc(1, size);
    r = uv_cwd(cwd, &size);
  }
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, cwd, NewStringType::kNormal).ToLocalChecked());
  free(cwd);
}

void Process::USleep(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  int microseconds = args[0]->IntegerValue(context).ToChecked();
  usleep(microseconds);
}

void on_exit(uv_process_t *req, int64_t exit_status, int term_signal) {
//fprintf(stderr, "on_exit\n");
  Isolate *isolate = Isolate::GetCurrent();
  Process *obj = (Process *)req->data;
  v8::HandleScope handleScope(isolate);
  Local<Value> argv[2] = { v8::BigInt::New(isolate, exit_status), v8::Integer::New(isolate, term_signal) };
  Local<Function> Callback = Local<Function>::New(isolate, obj->onExit);
  Local<Context> context = isolate->GetCurrentContext();
  uv_close((uv_handle_t*) req, NULL);
  Callback->Call(context, context->Global(), 2, argv);
}

void Process::Spawn(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));

  int written;
  uv_process_t* child_req = (uv_process_t*)calloc(1, sizeof(uv_process_t));
  uv_process_options_t options;
  uv_stdio_container_t child_stdio[3];

  memset(&options, 0, sizeof(uv_process_options_t));

  Local<String> filePath = args[0].As<String>();
  Local<String> cwd = args[1].As<String>();
  Local<Array> arguments = args[2].As<Array>();

  Socket* in = ObjectWrap::Unwrap<Socket>(args[3].As<v8::Object>());
  Socket* out = ObjectWrap::Unwrap<Socket>(args[4].As<v8::Object>());
  Socket* err = ObjectWrap::Unwrap<Socket>(args[5].As<v8::Object>());

  Process* obj = ObjectWrap::Unwrap<Process>(args.Holder());
  Local<Function> onExit = Local<Function>::Cast(args[6]);
  obj->onExit.Reset(isolate, onExit);

  int len = arguments->Length();
  char* argv[len + 2];
  argv[0] = (char*)calloc(1, filePath->Length());
  for (int i = 0; i < len; i++) {
    Local<String> val = arguments->Get(context, i).ToLocalChecked().As<v8::String>();
    argv[i + 1] = (char*)calloc(1, val->Length());
    val->WriteUtf8(isolate, argv[i + 1], val->Length(), &written, v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION);
  }
  argv[len + 1] = NULL;
  options.cwd = (char*)calloc(1, cwd->Length());
  options.file = (char*)calloc(1, filePath->Length());
  filePath->WriteUtf8(isolate, argv[0], filePath->Length(), &written, v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION);
  cwd->WriteUtf8(isolate, (char*)options.cwd, cwd->Length(), &written, v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION);
  options.stdio_count = 3;
  //options.flags |= UV_PROCESS_SETUID;
  //options.flags |= UV_PROCESS_SETGID;
  child_stdio[0].flags = UV_IGNORE;
  child_stdio[1].flags = UV_IGNORE;
  //child_stdio[2].flags = UV_IGNORE;
  child_stdio[0].flags = static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_READABLE_PIPE);
  child_stdio[0].data.stream = in->context->handle;
  child_stdio[1].flags = static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_WRITABLE_PIPE);
  child_stdio[1].data.stream = out->context->handle;
  child_stdio[2].flags = static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_WRITABLE_PIPE);
  child_stdio[2].data.stream = err->context->handle;
  options.stdio = child_stdio;
  options.exit_cb = on_exit;
  options.file = argv[0];
  options.args = argv;
  options.env = NULL;
  options.flags |= UV_PROCESS_WINDOWS_HIDE;
  options.flags |= UV_PROCESS_DETACHED;
  child_req->data = obj;
  int r = uv_spawn(env->loop, child_req, &options);
  if (r) {
    // will always be negative
    args.GetReturnValue().Set(Integer::New(isolate, r));
    return;
  }
  args.GetReturnValue().Set(Integer::New(isolate, child_req->pid));
}

void Process::NanoSleep(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  int seconds = args[0]->IntegerValue(context).ToChecked();
  int nanoseconds = args[1]->IntegerValue(context).ToChecked();
  struct timespec sleepfor;
  sleepfor.tv_sec = seconds;
  sleepfor.tv_nsec = nanoseconds;
  nanosleep(&sleepfor, NULL);
}

void Process::MemoryUsage(const FunctionCallbackInfo<Value> &args) {
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
}

void Process::CPUUsage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  uv_rusage_t rusage;
  int err = uv_getrusage(&rusage);
  if (err) {
    return args.GetReturnValue().Set(String::NewFromUtf8(isolate, uv_strerror(err), v8::NewStringType::kNormal).ToLocalChecked());
  }
  Local<Float64Array> array = args[0].As<Float64Array>();
  Local<ArrayBuffer> ab = array->Buffer();
  double *fields = static_cast<double *>(ab->GetContents().Data());
  fields[0] = MICROS_PER_SEC * rusage.ru_utime.tv_sec + rusage.ru_utime.tv_usec;
  fields[1] = MICROS_PER_SEC * rusage.ru_stime.tv_sec + rusage.ru_stime.tv_usec;
}

void Process::HRTime(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<ArrayBuffer> ab = args[0].As<BigUint64Array>()->Buffer();
  uint64_t *fields = static_cast<uint64_t *>(ab->GetContents().Data());
  fields[0] = uv_hrtime();
}

} // namespace process
} // namespace dv8

extern "C" {
	void* _register_process() {
		return (void*)dv8::process::InitAll;
	}
}
