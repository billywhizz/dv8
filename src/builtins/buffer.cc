#include <buffer.h>

namespace dv8
{

namespace builtins
{
using v8::Array;
using v8::ArrayBuffer;
using v8::ArrayBufferCreationMode;
using v8::Context;
using v8::Float64Array;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HeapStatistics;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Uint8Array;
using v8::Value;
using v8::EscapableHandleScope;
using v8::WeakCallbackInfo;

void Buffer::Init(Local<Object> exports)
{
  Isolate *isolate = exports->GetIsolate();
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

  tpl->SetClassName(String::NewFromUtf8(isolate, "Buffer"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "alloc", Buffer::Alloc);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "free", Buffer::Free);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "read", Buffer::Read);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "write", Buffer::Write);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "copy", Buffer::Copy);

  DV8_SET_EXPORT(isolate, tpl, "Buffer", exports);
}

void Buffer::New(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  HandleScope handle_scope(isolate);
  if (args.IsConstructCall()) {
    Buffer *obj = new Buffer();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  }
}

// TODO: figure out how to do shared buffers across threads
void Buffer::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
  Isolate *isolate = data.GetIsolate();
  v8::HandleScope handleScope(isolate);
  ObjectWrap *wrap = data.GetParameter();
  Buffer* b = static_cast<Buffer *>(wrap);
  isolate->AdjustAmountOfExternalAllocatedMemory((int64_t)b->_length * -1);
  //fprintf(stderr, "Buffer::Destroy\n");
  free(b->_data);
}

void Buffer::Alloc(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  EscapableHandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  int argc = args.Length();
  if (argc > 0) {
    uint32_t length = args[0]->Uint32Value(context).ToChecked();
    Buffer *b = ObjectWrap::Unwrap<Buffer>(args.Holder());
    b->_length = 0;
    if (length > 0)
    {
      b->_data = (char *)calloc(length, 1);
      if (b->_data == nullptr)
      {
        return;
      }
      Local<ArrayBuffer> ab = ArrayBuffer::New(isolate, b->_data, length, ArrayBufferCreationMode::kExternalized);
      b->_length = length;
      isolate->AdjustAmountOfExternalAllocatedMemory(length);
      args.GetReturnValue().Set(scope.Escape(ab));
    }
  } else {
    Buffer *b = ObjectWrap::Unwrap<Buffer>(args.Holder());
    Local<ArrayBuffer> ab = ArrayBuffer::New(isolate, b->_data, b->_length, ArrayBufferCreationMode::kExternalized);
    args.GetReturnValue().Set(scope.Escape(ab));
  }
}

void Buffer::Copy(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  uint32_t length = args[1]->Uint32Value(context).ToChecked();
  Buffer *source = ObjectWrap::Unwrap<Buffer>(args.Holder());
  Buffer *dest = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
  memcpy(dest->_data, source->_data, length);
}

void Buffer::Free(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Buffer *b = ObjectWrap::Unwrap<Buffer>(args.Holder());
  isolate->AdjustAmountOfExternalAllocatedMemory(b->_length * -1);
  free(b->_data);
}

void Buffer::Read(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  int32_t off = args[0]->Int32Value(context).ToChecked();
  int32_t len = args[1]->Int32Value(context).ToChecked();
  Buffer *b = ObjectWrap::Unwrap<Buffer>(args.Holder());
  const char *data = b->_data + off;
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, data, v8::String::kNormalString, len));
}

void Buffer::Write(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Local<String> str = args[0].As<String>();
  int length = str->Length();
  int32_t off = args[1]->Int32Value(context).ToChecked();
  Buffer *b = ObjectWrap::Unwrap<Buffer>(args.Holder());
  char *data = b->_data + off;
  int written;
  str->WriteUtf8(isolate, data, length, &written, v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION);
  args.GetReturnValue().Set(Integer::New(isolate, written));
}

} // namespace builtins
} // namespace dv8