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

  Persistent<Function> Process::constructor;

  void Process::Init(Local<Object> exports) {
    Isolate* isolate = exports->GetIsolate();
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

    tpl->SetClassName(String::NewFromUtf8(isolate, "Process"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "pid", Process::PID);

    constructor.Reset(isolate, tpl->GetFunction());
    DV8_SET_EXPORT(isolate, tpl, "Process", exports);
  }

  void Process::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    if (args.IsConstructCall()) {
      Local<Context> context = isolate->GetCurrentContext();
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

}
}