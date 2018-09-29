#include <process.h>

namespace dv8 {

namespace process {
  using v8::Context;
  using v8::Function;
  using v8::FunctionCallbackInfo;
  using v8::FunctionTemplate;
  using v8::Isolate;
  using v8::Local;
  using v8::Number;
  using v8::Integer;
  using v8::Object;
  using v8::Persistent;
  using v8::String;
  using v8::Value;
  using v8::Array;
  using v8::HeapStatistics;
  using v8::Float64Array;

  Persistent<Function> Process::constructor;

  void Process::Init(Local<Object> exports) {
    Isolate* isolate = exports->GetIsolate();
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

    tpl->SetClassName(String::NewFromUtf8(isolate, "Process"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "pid", Process::PID);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "memoryUsage", Process::MemoryUsage);

    constructor.Reset(isolate, tpl->GetFunction());
    DV8_SET_EXPORT(isolate, tpl, "Process", exports);
  }

  void Process::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    if (args.IsConstructCall()) {
      Process* obj = new Process();
      obj->Wrap(args.This());
      args.GetReturnValue().Set(args.This());
    } else {
      Local<Function> cons = Local<Function>::New(isolate, constructor);
      Local<Context> context = isolate->GetCurrentContext();
      Local<Object> instance = cons->NewInstance(context, 0, NULL).ToLocalChecked();
      args.GetReturnValue().Set(instance);
    }
  }

  void Process::NewInstance(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    const unsigned argc = 2;
    Local<Value> argv[argc] = { args[0], args[1] };
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
    args.GetReturnValue().Set(instance);
  }

  void Process::PID(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    args.GetReturnValue().Set(Integer::New(isolate, uv_os_getpid()));
  }

  void Process::MemoryUsage(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    size_t rss;
    int err = uv_resident_set_memory(&rss);
    if (err) {
      fprintf(stderr, "uv_error: %i", err);
      return;
    }
    HeapStatistics v8_heap_stats;
    isolate->GetHeapStatistics(&v8_heap_stats);
    //CHECK(args[0]->IsFloat64Array());
    Local<Float64Array> array = args[0].As<Float64Array>();
    //CHECK_EQ(array->Length(), 4);
    Local<ArrayBuffer> ab = array->Buffer();
    double* fields = static_cast<double*>(ab->GetContents().Data());
    fields[0] = rss;
    fields[1] = v8_heap_stats.total_heap_size();
    fields[2] = v8_heap_stats.used_heap_size();
    fields[3] = isolate->AdjustAmountOfExternalAllocatedMemory(0);
  }

}
}