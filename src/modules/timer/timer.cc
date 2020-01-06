#include <timer.h>

namespace dv8
{

namespace timer
{
using dv8::builtins::Environment;

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
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "again", Timer::Again);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "unref", Timer::UnRef);

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
        obj->handle = (uv_timer_t *)calloc(1, sizeof(uv_timer_t));
        int r = uv_timer_init(env->loop, obj->handle);
        obj->handle->data = obj;
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
    int r = uv_timer_start(t->handle, OnTimeout, timeout, 0);
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

void Timer::UnRef(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    Timer *t = ObjectWrap::Unwrap<Timer>(args.Holder());
    uv_unref((uv_handle_t*)t->handle);
}

void Timer::Stop(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    Timer *t = ObjectWrap::Unwrap<Timer>(args.Holder());
    t->onTimeout.Reset();
    int r = uv_timer_stop(t->handle);
    args.GetReturnValue().Set(Integer::New(isolate, r));
}

void Timer::Close(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    Timer *t = ObjectWrap::Unwrap<Timer>(args.Holder());
    uv_handle_t *handle = (uv_handle_t *)t->handle;
    uv_close(handle, OnClose);
    args.GetReturnValue().Set(Integer::New(isolate, 0));
}

void Timer::Again(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    Timer *t = ObjectWrap::Unwrap<Timer>(args.Holder());
    uv_handle_t *handle = (uv_handle_t *)t->handle;
    int r = uv_timer_again(t->handle);
    args.GetReturnValue().Set(Integer::New(isolate, r));
}

void Timer::OnTimeout(uv_timer_t *handle)
{
    Timer *t = (Timer *)handle->data;
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
}

} // namespace timer
} // namespace dv8

extern "C" {
	void* _register_timer() {
		return (void*)dv8::timer::InitAll;
	}
}
