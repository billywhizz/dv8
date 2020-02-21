#include "loop.h"

namespace dv8 {

namespace loop {
	using dv8::builtins::Environment;
	using dv8::builtins::Buffer;

	void InitAll(Local<Object> exports)
	{
		// initialise all the classes in this module
		// can also do any initial work here. will only be called once when 
		// library is loaded
		EventLoop::Init(exports);
		Isolate* isolate = exports->GetIsolate();
		DV8_SET_EXPORT_CONSTANT(isolate, String::NewFromUtf8(isolate, jsys_version_string()).ToLocalChecked(), "version", exports);
	}

	void EventLoop::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "EventLoop").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "listHandles", EventLoop::ListHandles);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "run", EventLoop::Run);
		DV8_SET_EXPORT(isolate, tpl, "EventLoop", exports);
	}

	void EventLoop::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			EventLoop* obj = new EventLoop();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void EventLoop::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		#if TRACE
		fprintf(stderr, "EventLoop::Destroy\n");
		#endif
	}

	void EventLoop::ListHandles(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		// in an instance method we can get the object instance calling this method
		jsys_loop* loop = env->loop;
		int ct_closing = 0;
		int ct_closed = 0;
		int ct_socket = 0;
		int ct_tty = 0;
		int ct_signal = 0;
		int ct_timer = 0;
		int ct_unknown = 0;
		for (int i = 0; i < loop->maxfds; i++) {
			jsys_descriptor* handle = loop->descriptors[i];
			if (handle != NULL) {
				ct_closing += handle->closing;
				ct_closed += handle->closed;
				switch(handle->type) {
					case JSYS_SOCKET:
						ct_socket++;
						break;
					case JSYS_TTY:
						ct_tty++;
						break;
					case JSYS_SIGNAL:
						ct_signal++;
						break;
					case JSYS_TIMER:
						ct_timer++;
						break;
					default:
						break;
				}
			}
		}
		Local<v8::Uint32Array> array = args[0].As<v8::Uint32Array>();
		Local<ArrayBuffer> ab = array->Buffer();
		int *fields = static_cast<int *>(ab->GetContents().Data());
		fields[0] = ct_socket;
		fields[1] = ct_tty;
		fields[2] = ct_signal;
		fields[3] = ct_timer;
		fields[4] = ct_unknown;
		fields[5] = ct_closing;
		fields[6] = ct_closed;
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
	void EventLoop::Run(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		int timeout = -1;
		if (args.Length() > 0) {
			timeout = args[0]->IntegerValue(context).ToChecked();
		}
		args.GetReturnValue().Set(Integer::New(isolate, jsys_loop_run_once(env->loop, timeout)));
	}

}
}	

extern "C" {
	void* _register_loop() {
		return (void*)dv8::loop::InitAll;
	}
}
