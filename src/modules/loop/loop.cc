#include "loop.h"

namespace dv8 {

namespace loop {

using dv8::builtins::Environment;
using dv8::builtins::Buffer;

	void InitAll(Local<Object> exports) {
		EventLoop::Init(exports);
	}

	void EventLoop::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "EventLoop").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stop", EventLoop::Stop);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "run", EventLoop::Run);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "isAlive", EventLoop::IsAlive);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "close", EventLoop::Close);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "reset", EventLoop::Reset);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "error", EventLoop::Error);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onIdle", EventLoop::OnIdle);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onPrepare", EventLoop::OnPrepare);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onCheck", EventLoop::OnCheck);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "unref", EventLoop::UnRef);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "ref", EventLoop::Ref);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "listHandles", EventLoop::ListHandles);

		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, UV_RUN_DEFAULT), "UV_RUN_DEFAULT", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, UV_RUN_ONCE), "UV_RUN_ONCE", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, UV_RUN_NOWAIT), "UV_RUN_NOWAIT", exports);

		DV8_SET_EXPORT(isolate, tpl, "EventLoop", exports);
	}

	void EventLoop::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		if (args.IsConstructCall()) {
			EventLoop* obj = new EventLoop();

			obj->idle_handle = (uv_idle_t*)calloc(sizeof(uv_idle_t), 1);
			obj->idle_handle->data = obj;
			uv_idle_init(env->loop, obj->idle_handle);
			uv_unref((uv_handle_t*)obj->idle_handle);

			obj->check_handle = (uv_check_t*)calloc(sizeof(uv_check_t), 1);
			obj->check_handle->data = obj;
			uv_check_init(env->loop, obj->check_handle);
			uv_unref((uv_handle_t*)obj->check_handle);

			obj->prepare_handle = (uv_prepare_t*)calloc(sizeof(uv_prepare_t), 1);
			obj->prepare_handle->data = obj;
			uv_prepare_init(env->loop, obj->prepare_handle);
			uv_unref((uv_handle_t*)obj->prepare_handle);

			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void EventLoop::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		Isolate *isolate = data.GetIsolate();
		v8::HandleScope handleScope(isolate);
		ObjectWrap *wrap = data.GetParameter();
		EventLoop* obj = static_cast<EventLoop *>(wrap);
		fprintf(stderr, "EventLoop::Destroy");
		uv_close((uv_handle_t*)obj->prepare_handle, OnClose);
		uv_close((uv_handle_t*)obj->check_handle, OnClose);
		uv_close((uv_handle_t*)obj->idle_handle, OnClose);
	}

	void EventLoop::Error(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		int r = args[1]->IntegerValue(context).ToChecked();
		const char *error = uv_strerror(r);
		args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), error, NewStringType::kNormal).ToLocalChecked());
	}

	void EventLoop::Stop(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		EventLoop* obj = ObjectWrap::Unwrap<EventLoop>(args.Holder());
		uv_stop(env->loop);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
	void EventLoop::Run(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		EventLoop* obj = ObjectWrap::Unwrap<EventLoop>(args.Holder());
		int mode = args[0]->IntegerValue(context).ToChecked();
		int status = uv_run(env->loop, (uv_run_mode)mode);
		args.GetReturnValue().Set(Integer::New(isolate, status));
	}
	
	void EventLoop::Ref(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		EventLoop* obj = ObjectWrap::Unwrap<EventLoop>(args.Holder());
		uv_ref((uv_handle_t*)obj->idle_handle);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
	void EventLoop::ListHandles(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		EventLoop* obj = ObjectWrap::Unwrap<EventLoop>(args.Holder());
		Buffer *b = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
		size_t len = b->_length;
		uint8_t *work = (uint8_t *)b->_data;
		loophandle_t handles;
		handles.off = 0;
		handles.size = 0;
		handles.buf = work;
		uv_walk(env->loop, [](uv_handle_t *handle, void *arg) {
			loophandle_t* handles = (loophandle_t*)arg;
			uint8_t *work = handles->buf + handles->off;
			work[0] = uv_is_active(handle);
			work++;
			handles->off++;
			const char* type = uv_handle_type_name(handle->type);
			int typelen = 0;
			if (type == NULL) {

			} else {
				typelen = strlen(type);
			}
			work[0] = typelen;
			work++;
			handles->off++;
			if (typelen > 0) {
				memcpy(work, type, typelen);
				work += typelen;
				handles->off += typelen;
			}
			handles->size++;
		}, &handles);
		work = handles.buf + handles.off;
		work[0] = 255;
		args.GetReturnValue().Set(Integer::New(isolate, handles.size));
	}
	
	void EventLoop::UnRef(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		EventLoop* obj = ObjectWrap::Unwrap<EventLoop>(args.Holder());
		uv_unref((uv_handle_t*)obj->idle_handle);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
	void EventLoop::IsAlive(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		EventLoop* obj = ObjectWrap::Unwrap<EventLoop>(args.Holder());
		int alive = uv_loop_alive(env->loop);
		args.GetReturnValue().Set(Integer::New(isolate, alive));
	}
	
	void EventLoop::OnClose(uv_handle_t *handle)
	{
			free(handle);
	}

	void EventLoop::Close(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		EventLoop* obj = ObjectWrap::Unwrap<EventLoop>(args.Holder());
		int ok = uv_loop_close(env->loop);
		args.GetReturnValue().Set(Integer::New(isolate, ok));
	}
	
	void EventLoop::Reset(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		EventLoop* obj = ObjectWrap::Unwrap<EventLoop>(args.Holder());
		uv_close((uv_handle_t*)obj->prepare_handle, OnClose);
		uv_close((uv_handle_t*)obj->check_handle, OnClose);
		uv_close((uv_handle_t*)obj->idle_handle, OnClose);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
	void on_idle(uv_idle_t* handle) {
		Isolate *isolate = Isolate::GetCurrent();
		EventLoop *obj = (EventLoop *)handle->data;
		v8::HandleScope handleScope(isolate);
		if (obj->callbacks.onIdle == 1) {
				Local<Value> argv[0] = {};
				Local<Function> Callback = Local<Function>::New(isolate, obj->onIdle);
				v8::TryCatch try_catch(isolate);
				Local<Context> context = isolate->GetCurrentContext();
				Callback->Call(context, context->Global(), 0, argv);
				if (try_catch.HasCaught()) {
					dv8::ReportException(isolate, &try_catch);
				}
		}
	}

	void on_prepare(uv_prepare_t* handle) {
		Isolate *isolate = Isolate::GetCurrent();
		EventLoop *obj = (EventLoop *)handle->data;
		v8::HandleScope handleScope(isolate);
        if (obj->callbacks.onPrepare == 1) {
            Local<Value> argv[0] = {};
            Local<Function> Callback = Local<Function>::New(isolate, obj->onPrepare);
						Local<Context> context = isolate->GetCurrentContext();
            Callback->Call(context, context->Global(), 0, argv);
        }
	}

	void on_check(uv_check_t* handle) {
		Isolate *isolate = Isolate::GetCurrent();
		EventLoop *obj = (EventLoop *)handle->data;
		v8::HandleScope handleScope(isolate);
        if (obj->callbacks.onCheck == 1) {
            Local<Value> argv[0] = {};
            Local<Function> Callback = Local<Function>::New(isolate, obj->onCheck);
						Local<Context> context = isolate->GetCurrentContext();
            Callback->Call(context, context->Global(), 0, argv);
        }
	}

	void EventLoop::OnIdle(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
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
	
	void EventLoop::OnCheck(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		EventLoop* obj = ObjectWrap::Unwrap<EventLoop>(args.Holder());
		int argc = args.Length();
		if (argc > 0) {
			Local<Function> onCheck = Local<Function>::Cast(args[0]);
			uv_check_start(obj->check_handle, on_check);
			obj->onCheck.Reset(isolate, onCheck);
			obj->callbacks.onCheck = 1;
		} else {
			uv_check_stop(obj->check_handle);
			obj->onCheck.Reset();
			obj->callbacks.onCheck = 0;
		}
	}
	
	void EventLoop::OnPrepare(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		EventLoop* obj = ObjectWrap::Unwrap<EventLoop>(args.Holder());
		int argc = args.Length();
		if (argc > 0) {
			Local<Function> onPrepare = Local<Function>::Cast(args[0]);
			uv_prepare_start(obj->prepare_handle, on_prepare);
			obj->onPrepare.Reset(isolate, onPrepare);
			obj->callbacks.onPrepare = 1;
		} else {
			uv_prepare_stop(obj->prepare_handle);
			obj->onPrepare.Reset();
			obj->callbacks.onPrepare = 0;
		}
	}
	
}
}	

extern "C" {
	void* _register_loop() {
		return (void*)dv8::loop::InitAll;
	}
}
