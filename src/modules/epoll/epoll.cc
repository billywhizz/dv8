#include "epoll.h"

namespace dv8 {

namespace epoll {
	using dv8::builtins::Environment;
	using dv8::builtins::Buffer;

	void InitAll(Local<Object> exports)
	{
		// initialise all the classes in this module
		// can also do any initial work here. will only be called once when 
		// library is loaded
		Epoll::Init(exports);
		Isolate* isolate = exports->GetIsolate();
		DV8_SET_EXPORT_CONSTANT(isolate, String::NewFromUtf8(isolate, jsys_version_string()).ToLocalChecked(), "version", exports);
	}

	void Epoll::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		// create a function template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "Epoll").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
		// an instance method
    DV8_SET_METHOD(isolate, tpl, "listHandles", Epoll::ListHandles);
		// a static method on the template
		DV8_SET_METHOD(isolate, tpl, "run", Epoll::Run);
		// store the template on the exports object
		DV8_SET_EXPORT(isolate, tpl, "Epoll", exports);
	}

	void Epoll::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			//HandleScope handle_scope(isolate);
			Epoll* obj = new Epoll();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void Epoll::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		#if TRACE
		fprintf(stderr, "Epoll::Destroy\n");
		#endif
	}

	void Epoll::ListHandles(const FunctionCallbackInfo<Value> &args)
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
	
	void Epoll::Run(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		int r = jsys_loop_run(env->loop);
		args.GetReturnValue().Set(Integer::New(isolate, r));
	}

}
}	

extern "C" {
	void* _register_epoll() {
		return (void*)dv8::epoll::InitAll;
	}
}
