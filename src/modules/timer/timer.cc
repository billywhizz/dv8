#include <timer.h>

namespace dv8
{

namespace timer
{
using dv8::builtins::Environment;
using v8::Array;
using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;

void Timer::Init(Local<Object> exports)
{
    Isolate *isolate = exports->GetIsolate();
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

    tpl->SetClassName(String::NewFromUtf8(isolate, "Timer"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "start", Timer::Start);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stop", Timer::Stop);

    DV8_SET_EXPORT(isolate, tpl, "Timer", exports);
}

void Timer::New(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    if (args.IsConstructCall()) {
        Timer *obj = new Timer();
        obj->handle = (uv_timer_t *)calloc(1, sizeof(uv_timer_t));
        obj->handle->data = obj;
        obj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    }
}

void Timer::Start(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    Local<Context> context = isolate->GetCurrentContext();
    Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(32));
    Local<Function> onTimeout = Local<Function>::Cast(args[0]);
    Timer *t = ObjectWrap::Unwrap<Timer>(args.Holder());
    t->onTimeout.Reset(isolate, onTimeout);
    int timeout = args[1]->Int32Value(context).ToChecked();
    int r = uv_timer_init(env->loop, t->handle);
    r = uv_timer_start(t->handle, OnTimeout, timeout, 0);
    if (args.Length() > 2)
    {
        uv_timer_set_repeat(t->handle, args[2]->Int32Value(context).ToChecked());
    }
    args.GetReturnValue().Set(Integer::New(isolate, r));
}

void Timer::OnClose(uv_handle_t *handle)
{
    free(handle);
}

void Timer::Stop(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    Timer *t = ObjectWrap::Unwrap<Timer>(args.Holder());
    int r = uv_timer_stop(t->handle);
    uv_handle_t *handle = (uv_handle_t *)t->handle;
    uv_close(handle, OnClose);
    t->onTimeout.Reset();
    args.GetReturnValue().Set(Integer::New(isolate, r));
}

void Timer::OnTimeout(uv_timer_t *handle)
{
    Timer *t = (Timer *)handle->data;
    Isolate *isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    const unsigned int argc = 0;
    Local<Value> argv[argc] = {};
    Local<Function> foo = Local<Function>::New(isolate, t->onTimeout);
    v8::TryCatch try_catch(isolate);
    foo->Call(isolate->GetCurrentContext()->Global(), 0, argv);
    if (try_catch.HasCaught()) {
        dv8::ReportException(isolate, &try_catch);
    }
}

} // namespace timer
} // namespace dv8