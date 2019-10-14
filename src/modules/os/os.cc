#include "os.h"

namespace dv8
{

namespace os
{
using dv8::builtins::Environment;

void InitAll(Local<Object> exports)
{
  OS::Init(exports);
}

void OS::Init(Local<Object> exports)
{
  Isolate *isolate = exports->GetIsolate();
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

  tpl->SetClassName(String::NewFromUtf8(isolate, "OS"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onSignal", OS::OnSignal);

  DV8_SET_EXPORT(isolate, tpl, "OS", exports);
}

void OS::New(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  HandleScope handle_scope(isolate);
  if (args.IsConstructCall()) {
    OS *obj = new OS();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  }
}

void OS::OnSignal(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(32));
  OS *os = ObjectWrap::Unwrap<OS>(args.Holder());
  if (args.Length() > 0)
  {
    if (args[0]->IsFunction())
    {
      Local<Function> onSignal = Local<Function>::Cast(args[0]);
      os->_onSignal.Reset(isolate, onSignal);
      uv_signal_t *signalHandle = new uv_signal_t;
      signalHandle->data = os;
      int r = uv_signal_init(env->loop, signalHandle);
      int sigmask = 15;
      if (args.Length() > 1)
      {
        Local<Context> context = isolate->GetCurrentContext();
        sigmask = args[1]->Uint32Value(context).ToChecked();
      }
      r = uv_signal_start(signalHandle, on_signal, sigmask);
      args.GetReturnValue().Set(Integer::New(isolate, r));
    }
  }
}

void OS::on_signal(uv_signal_t *handle, int signum)
{
  Isolate *isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  OS *os = (OS *)handle->data;
  Local<Value> argv[1] = {Number::New(isolate, signum)};
  Local<Function> Callback = Local<Function>::New(isolate, os->_onSignal);
  Local<Value> ret = Callback->Call(context, context->Global(), 1, argv).ToLocalChecked();
  uint32_t close = ret->Uint32Value(context).ToChecked();
  if (close == 1)
  {
    uv_signal_stop(handle);
    fprintf(stderr, "singal watcher stopped: %i\n", signum);
  }
}

} // namespace os
} // namespace dv8

extern "C" {
	void* _register_os() {
		return (void*)dv8::os::InitAll;
	}
}
