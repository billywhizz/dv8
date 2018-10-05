#include <buffer.h>

namespace dv8 {

namespace builtins {
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
  using v8::ArrayBufferCreationMode;
  using v8::ArrayBuffer;
  using v8::MaybeLocal;
  using v8::Uint8Array;

  Persistent<Function> Buffer::constructor;

  void Buffer::Init(Local<Object> exports) {
    Isolate* isolate = exports->GetIsolate();
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

    tpl->SetClassName(String::NewFromUtf8(isolate, "Buffer"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "alloc", Buffer::Alloc);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "pull", Buffer::Pull);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "push", Buffer::Push);

    constructor.Reset(isolate, tpl->GetFunction());
    DV8_SET_EXPORT(isolate, tpl, "Buffer", exports);
  }

  void Buffer::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    if (args.IsConstructCall()) {
      Buffer* obj = new Buffer();
      obj->Wrap(args.This());
      args.GetReturnValue().Set(args.This());
    } else {
      Local<Function> cons = Local<Function>::New(isolate, constructor);
      Local<Context> context = isolate->GetCurrentContext();
      Local<Object> instance = cons->NewInstance(context, 0, NULL).ToLocalChecked();
      args.GetReturnValue().Set(instance);
    }
  }

  void Buffer::NewInstance(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    const unsigned argc = 2;
    Local<Value> argv[argc] = { args[0], args[1] };
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
    args.GetReturnValue().Set(instance);
  }

  void Buffer::Alloc(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    Local<Context> context = isolate->GetCurrentContext();
    uint32_t length = args[0]->Uint32Value(context).ToChecked();
    Buffer* b = ObjectWrap::Unwrap<Buffer>(args.Holder());
    b->_length = 0;
    if (length > 0) {
      b->_data = (char*)calloc(length, 1);
      if (b->_data == nullptr) {
        return;
      }
      Local<ArrayBuffer> ab = ArrayBuffer::New(isolate, b->_data, length, ArrayBufferCreationMode::kInternalized);
      b->ab.Reset(isolate, ab);
      //isolate->AdjustAmountOfExternalAllocatedMemory(length);
      args.GetReturnValue().Set(ab);
      b->_length = length;
    }
  }

  void Buffer::Pull(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    Local<Context> context = isolate->GetCurrentContext();
    int32_t off = args[0]->Int32Value(context).ToChecked();
    int32_t len = args[1]->Int32Value(context).ToChecked();
    Buffer* b = ObjectWrap::Unwrap<Buffer>(args.Holder());
    const char* data = b->_data + off;
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, data, v8::String::kNormalString, len));
  }

  void Buffer::Push(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    Local<Context> context = isolate->GetCurrentContext();
    Local<String> str = args[0].As<String>();
    int length = str->Length();
    int32_t off = args[1]->Int32Value(context).ToChecked();
    Buffer* b = ObjectWrap::Unwrap<Buffer>(args.Holder());
    char* data = b->_data + off;
    int written;
    str->WriteUtf8(isolate, data, length, &written, v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION);
    args.GetReturnValue().Set(Integer::New(isolate, written));
  }

}
}