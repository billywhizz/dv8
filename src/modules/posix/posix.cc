#include "posix.h"

namespace dv8 {

namespace posix {

	void Queue::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "Queue"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", Queue::Setup);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "open", Queue::Open);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "send", Queue::Send);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "recv", Queue::Recv);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "close", Queue::Close);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "unlink", Queue::Unlink);
	
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, CONSUMER), "CONSUMER", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, PRODUCER), "PRODUCER", exports);

		DV8_SET_EXPORT(isolate, tpl, "Queue", exports);
	}

	void Queue::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			Queue* obj = new Queue();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void Queue::Setup(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		Queue* obj = ObjectWrap::Unwrap<Queue>(args.Holder());
		Buffer *b = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
		obj->data = (char *)b->_data;
		obj->attr.mq_flags = 0;
		obj->attr.mq_maxmsg = 1000;
		obj->attr.mq_msgsize = b->_length;
		obj->attr.mq_curmsgs = 0;
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
	void Queue::Open(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		Queue* obj = ObjectWrap::Unwrap<Queue>(args.Holder());
    String::Utf8Value str(args.GetIsolate(), args[0]);
		uint32_t type = args[1]->Uint32Value(context).ToChecked();
		const char *path = *str;
		obj->sockName = path;
		if (type == PRODUCER) {
			obj->mq = mq_open(path, O_CREAT | O_WRONLY, 0644, &obj->attr);
			//obj->mq = mq_open(path, O_CREAT | O_WRONLY | O_NONBLOCK, 0644, &obj->attr);
		} else {
			obj->mq = mq_open(path, O_RDONLY, 0644, &obj->attr);
			//obj->mq = mq_open(path, O_RDONLY | O_NONBLOCK, 0644, &obj->attr);
		}
		args.GetReturnValue().Set(Integer::New(isolate, obj->mq));
	}
	
	void Queue::Send(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		Queue* obj = ObjectWrap::Unwrap<Queue>(args.Holder());
		uint32_t len = args[0]->Uint32Value(context).ToChecked();
		uint32_t status = mq_send(obj->mq, obj->data, len, 0);
		if (status == 0) {
			args.GetReturnValue().Set(Integer::New(isolate, len));
			return;
		}
		fprintf(stderr, "errno: %i\n", errno);
		args.GetReturnValue().Set(Integer::New(isolate, status));
	}
	
	void Queue::Recv(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		Queue* obj = ObjectWrap::Unwrap<Queue>(args.Holder());
		uint32_t len = args[0]->Uint32Value(context).ToChecked();
		ssize_t bytes_read = mq_receive(obj->mq, obj->data, len, NULL);
		args.GetReturnValue().Set(Integer::New(isolate, bytes_read));
	}
	
	void Queue::Close(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		Queue* obj = ObjectWrap::Unwrap<Queue>(args.Holder());
    mq_close(obj->mq);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
	void Queue::Unlink(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		Queue* obj = ObjectWrap::Unwrap<Queue>(args.Holder());
    mq_unlink(obj->sockName);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
}
}	
