#include <process.h>

namespace dv8 {

namespace process {
using dv8::builtins::Environment;
using v8::Array;
using v8::BigUint64Array;
using v8::Context;
using v8::Float64Array;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HeapSpaceStatistics;
using v8::HeapStatistics;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;

void Process::Init(Local<Object> exports) {
  Isolate *isolate = exports->GetIsolate();
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Process"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "pid", Process::PID);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "memoryUsage", Process::MemoryUsage);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "heapUsage", Process::HeapSpaceUsage);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "cpuUsage", Process::CPUUsage);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "hrtime", Process::HRTime);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "sleep", Process::Sleep);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "cwd", Process::Cwd);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "usleep", Process::USleep);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "runMicroTasks", Process::RunMicroTasks);
  // spawn, kill, getTitle, setTitle, rss, uptime, rusage, ppid, interfaces, loadavg, exepath, cwd, chdir, homedir, tmpdir, passwd, memory, handles, hostname, getPriority, setPriority 
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
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(32));
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
    o->Set(String::NewFromUtf8(isolate, s.space_name(), v8::NewStringType::kNormal).ToLocalChecked(), array);
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

void Process::MemoryUsage(const FunctionCallbackInfo<Value> &args) {
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

void Process::CPUUsage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  uv_rusage_t rusage;
  int err = uv_getrusage(&rusage);
  if (err) {
    return args.GetReturnValue().Set(String::NewFromUtf8(isolate, uv_strerror(err), v8::String::kNormalString));
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