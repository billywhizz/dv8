#include <timer.h>

namespace dv8
{

namespace timer
{
using dv8::builtins::Environment;

int on_timer_event(jsys_descriptor *timer) {
    jsys_timer_read(timer);
    Timer *t = (Timer *)timer->data;
    Isolate *isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    const unsigned int argc = 0;
    Local<Value> argv[argc] = {};
    v8::TryCatch try_catch(isolate);
    Local<Function> cb = Local<Function>::New(isolate, t->onTimeout);
    Local<Context> ctx = isolate->GetCurrentContext();
    cb->Call(ctx, ctx->Global(), 0, argv);
    if (try_catch.HasCaught()) {
        dv8::ReportException(isolate, &try_catch);
    }
    return 0;
}

void InitAll(Local<Object> exports)
{
  Timer::Init(exports);
}

void Timer::Init(Local<Object> exports)
{
    Isolate *isolate = exports->GetIsolate();
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

    tpl->SetClassName(String::NewFromUtf8(isolate, "Timer").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "start", Timer::Start);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stop", Timer::Stop);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "close", Timer::Close);

    DV8_SET_EXPORT(isolate, tpl, "Timer", exports);
}

void Timer::New(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    if (args.IsConstructCall()) {
        Local<Context> context = isolate->GetCurrentContext();
        Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
        Timer *obj = new Timer();
        obj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    }
}

void Timer::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
    Isolate *isolate = data.GetIsolate();
    v8::HandleScope handleScope(isolate);
    ObjectWrap *wrap = data.GetParameter();
    Timer* sock = static_cast<Timer *>(wrap);
    #if TRACE
    fprintf(stderr, "Timer::Destroy\n");
    #endif
}

void Timer::Start(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    Local<Context> context = isolate->GetCurrentContext();
    Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
    Local<Function> onTimeout = Local<Function>::Cast(args[0]);
    Timer *t = ObjectWrap::Unwrap<Timer>(args.Holder());
    t->onTimeout.Reset(isolate, onTimeout);
    int timeout = args[1]->Int32Value(context).ToChecked();
    t->handle = jsys_timer_create(env->loop, timeout * 1000000, timeout * 1000000);
    t->handle->callback = dv8::timer::on_timer_event;
    t->handle->data = t;
    int r = jsys_loop_add(env->loop, t->handle);
    args.GetReturnValue().Set(Integer::New(isolate, r));
}

void Timer::Stop(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    Timer *t = ObjectWrap::Unwrap<Timer>(args.Holder());
    t->onTimeout.Reset();
    int r = jsys_timer_reset(t->handle, 0, 0);
    args.GetReturnValue().Set(Integer::New(isolate, r));
}

void Timer::Close(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    Timer *t = ObjectWrap::Unwrap<Timer>(args.Holder());
    jsys_descriptor_free(t->handle);
    args.GetReturnValue().Set(Integer::New(isolate, 0));
}

} // namespace timer
} // namespace dv8

extern "C" {
	void* _register_timer() {
		return (void*)dv8::timer::InitAll;
	}
}
