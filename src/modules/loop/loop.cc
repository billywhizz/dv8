#include "loop.h"

namespace dv8 {

namespace loop {
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
	using dv8::builtins::Environment;

	void EventLoop::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "EventLoop"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stop", EventLoop::Stop);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "run", EventLoop::Run);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "isAlive", EventLoop::IsAlive);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "close", EventLoop::Close);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onIdle", EventLoop::OnIdle);

		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, UV_RUN_DEFAULT), "UV_RUN_DEFAULT", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, UV_RUN_ONCE), "UV_RUN_ONCE", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, UV_RUN_NOWAIT), "UV_RUN_NOWAIT", exports);

		DV8_SET_EXPORT(isolate, tpl, "EventLoop", exports);
	}

	void EventLoop::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		if (args.IsConstructCall()) {
			EventLoop* obj = new EventLoop();
			obj->idle_handle = (uv_idle_t*)calloc(sizeof(uv_idle_t), 1);
			obj->idle_handle->data = obj;
			uv_idle_init(env->loop, obj->idle_handle);
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void EventLoop::Stop(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		EventLoop* obj = ObjectWrap::Unwrap<EventLoop>(args.Holder());
		uv_stop(env->loop);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
	void EventLoop::Run(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		EventLoop* obj = ObjectWrap::Unwrap<EventLoop>(args.Holder());
		int mode = args[0]->IntegerValue(context).ToChecked();
		uv_run(env->loop, (uv_run_mode)mode);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
	void EventLoop::IsAlive(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		EventLoop* obj = ObjectWrap::Unwrap<EventLoop>(args.Holder());
		int alive = uv_loop_alive(env->loop);
		args.GetReturnValue().Set(Integer::New(isolate, alive));
	}
	
	void EventLoop::Close(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		EventLoop* obj = ObjectWrap::Unwrap<EventLoop>(args.Holder());
		int ok = uv_loop_close(env->loop);
		args.GetReturnValue().Set(Integer::New(isolate, ok));
	}
	
	void on_idle(uv_idle_t* handle) {
		Isolate *isolate = Isolate::GetCurrent();
		EventLoop *obj = (EventLoop *)handle->data;
        if (obj->callbacks.onIdle == 1) {
            Local<Value> argv[0] = {};
            Local<Function> Callback = Local<Function>::New(isolate, obj->onIdle);
            Callback->Call(isolate->GetCurrentContext()->Global(), 0, argv);
        }
	}

	void EventLoop::OnIdle(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		EventLoop* obj = ObjectWrap::Unwrap<EventLoop>(args.Holder());
		int argc = args.Length();
		if (argc > 0) {
			Local<Function> onIdle = Local<Function>::Cast(args[0]);
			uv_idle_start(obj->idle_handle, on_idle);
			obj->onIdle.Reset(isolate, onIdle);
			obj->callbacks.onIdle = 1;
		} else {
			uv_idle_stop(obj->idle_handle);
			obj->onIdle.Reset();
			obj->callbacks.onIdle = 0;
		}
	}
	
}
}	
