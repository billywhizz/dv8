#include <timer.h>

namespace dv8
{

namespace builtins
{
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

Persistent<Function> Timer::constructor;

void Timer::Init(Local<Object> exports)
{
    Isolate *isolate = exports->GetIsolate();
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

    tpl->SetClassName(String::NewFromUtf8(isolate, "Timer"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "start", Timer::Start);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stop", Timer::Stop);

    constructor.Reset(isolate, tpl->GetFunction());
    DV8_SET_EXPORT(isolate, tpl, "Timer", exports);
}

void Timer::New(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    if (args.IsConstructCall())
    {
        Local<Context> context = isolate->GetCurrentContext();
        Timer *obj = new Timer();
        obj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    }
    else
    {
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Object> instance = cons->NewInstance(context, 0, NULL).ToLocalChecked();
        args.GetReturnValue().Set(instance);
    }
}

void Timer::NewInstance(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    const unsigned argc = 2;
    Local<Value> argv[argc] = {args[0], args[1]};
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
    args.GetReturnValue().Set(instance);
}

void Timer::Start(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> onTimeout = Local<Function>::Cast(args[0]);
    int timeout = args[1]->Int32Value(context).ToChecked();
    uv_timer_t handle;
    int r = uv_timer_init(uv_default_loop(), &handle);
    fprintf(stderr, "uv_timer_init: %i\n", r);
    r = uv_timer_start(&handle, OnTimeout, timeout, 0);
    fprintf(stderr, "uv_timer_start: %i\n", r);
    args.GetReturnValue().Set(Integer::New(isolate, uv_os_getpid()));
}

void Timer::Stop(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    Local<Context> context = isolate->GetCurrentContext();
}

static void OnTimeout(uv_timer_t *handle)
{
    //Local<Function> foo = Local<Function>::New(isolate, obj->_onConnect);
    //foo->Call(isolate->GetCurrentContext()->Global(), 1, argv);
/*
uv_handle_t* const handle_;

int r = uv_timer_init(env->event_loop(), &handle_);
int err = uv_timer_start(&wrap->handle_, OnTimeout, timeout, 0);
    int err = uv_timer_stop(&wrap->handle_);

static void OnTimeout(uv_timer_t* handle) {
    TimerWrap* wrap = static_cast<TimerWrap*>(handle->data);
    ret = wrap->MakeCallback(env->timers_callback_function(), 1, args);

*/
}

} // namespace builtins
} // namespace dv8