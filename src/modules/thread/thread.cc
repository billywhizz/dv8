#include "thread.h"

namespace dv8
{

namespace thread
{
using dv8::builtins::Buffer;
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
using v8::ArrayBufferCreationMode;

Persistent<Function> Thread::constructor;

void start_context(uv_work_t *req)
{
	thread_handle *th = (thread_handle *)req->data;
	v8::Isolate::CreateParams create_params;
	create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	v8::Isolate *isolate = v8::Isolate::New(create_params);
	{
		isolate->SetAbortOnUncaughtExceptionCallback(dv8::ShouldAbortOnUncaughtException);
		isolate->SetFatalErrorHandler(dv8::OnFatalError);
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope handle_scope(isolate);

		Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
		global->Set(String::NewFromUtf8(isolate, "version", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Version));
		global->Set(String::NewFromUtf8(isolate, "print", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Print));
		global->Set(String::NewFromUtf8(isolate, "module", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, LoadModule));
		global->Set(String::NewFromUtf8(isolate, "require", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Require));
		v8::Local<v8::Context> context = Context::New(isolate, NULL, global);
		if (context.IsEmpty())
		{
			fprintf(stderr, "Error creating context\n");
			return;
		}
		const char *str = "./foo.js";
		v8::Context::Scope context_scope(context);
		v8::Local<v8::Object> globalInstance = context->Global();
		dv8::builtins::Buffer::Init(globalInstance);
		globalInstance->Set(v8::String::NewFromUtf8(isolate, "global", v8::NewStringType::kNormal).ToLocalChecked(), globalInstance);
		globalInstance->Set(String::NewFromUtf8(isolate, "workerData", v8::NewStringType::kNormal).ToLocalChecked(), ArrayBuffer::New(isolate, th->data, th->length, ArrayBufferCreationMode::kExternalized));
		v8::TryCatch try_catch(isolate);
		v8::MaybeLocal<v8::String> source = dv8::ReadFile(isolate, str);
		if (try_catch.HasCaught())
		{
			dv8::DecorateErrorStack(isolate, try_catch);
			return;
		}
		v8::MaybeLocal<v8::Script> script = v8::Script::Compile(context, source.ToLocalChecked());
		if (try_catch.HasCaught())
		{
			dv8::DecorateErrorStack(isolate, try_catch);
			return;
		}
		script.ToLocalChecked()->Run(context);
		if (try_catch.HasCaught())
		{
			dv8::DecorateErrorStack(isolate, try_catch);
			return;
		}
		int alive;
/*
		do
		{
			uv_run(uv_default_loop(), UV_RUN_DEFAULT);
			alive = uv_loop_alive(uv_default_loop());
			if (alive != 0)
			{
				continue;
			}
			alive = uv_loop_alive(uv_default_loop());
		} while (alive != 0);
*/
		uv_loop_t* loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
		uv_loop_init(loop);
		do
		{
			uv_run(loop, UV_RUN_DEFAULT);
			alive = uv_loop_alive(loop);
			if (alive != 0)
			{
				continue;
			}
			alive = uv_loop_alive(loop);
		} while (alive != 0);
		int r = uv_loop_close(loop);
		if (r != 0) {
			fprintf(stderr, "uv_thread_loop_close: %i\n", r);
		}
	}
	isolate->Dispose();
	delete create_params.array_buffer_allocator;
}

void on_context_complete(uv_work_t *req, int status)
{
	thread_handle *th = (thread_handle *)req->data;
	Thread *t = (Thread *)th->object;
	Isolate *isolate = Isolate::GetCurrent();
	v8::HandleScope handleScope(isolate);
	const unsigned int argc = 0;
	Local<Value> argv[argc] = {};
	Local<Function> foo = Local<Function>::New(isolate, t->onComplete);
	v8::TryCatch try_catch(isolate);
	foo->Call(isolate->GetCurrentContext()->Global(), 0, argv);
	if (try_catch.HasCaught())
	{
		DecorateErrorStack(isolate, try_catch);
	}
}

void Thread::Init(Local<Object> exports)
{
	Isolate *isolate = exports->GetIsolate();
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

	tpl->SetClassName(String::NewFromUtf8(isolate, "Thread"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "start", Thread::Start);

	constructor.Reset(isolate, tpl->GetFunction());
	DV8_SET_EXPORT(isolate, tpl, "Thread", exports);
}

void Thread::New(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	HandleScope handle_scope(isolate);
	if (args.IsConstructCall())
	{
		Thread *obj = new Thread();
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

void Thread::NewInstance(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	const unsigned argc = 2;
	Local<Value> argv[argc] = {args[0], args[1]};
	Local<Function> cons = Local<Function>::New(isolate, constructor);
	Local<Context> context = isolate->GetCurrentContext();
	Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
	args.GetReturnValue().Set(instance);
}

void Thread::Start(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	v8::HandleScope handleScope(isolate);
	Thread *obj = ObjectWrap::Unwrap<Thread>(args.Holder());
	Buffer *b = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
	Local<Function> onComplete = Local<Function>::Cast(args[1]);
	obj->onComplete.Reset(isolate, onComplete);
	size_t len = b->_length;
	obj->handle = (uv_work_t *)malloc(sizeof(uv_work_t));
	thread_handle *th = (thread_handle *)malloc(sizeof(thread_handle));
	th->data = b->_data;
	th->length = b->_length;
	th->object = (void *)obj;
	obj->handle->data = (void *)th;
	uv_queue_work(uv_default_loop(), obj->handle, start_context, on_context_complete);
	args.GetReturnValue().Set(Integer::New(isolate, 0));
}

} // namespace thread
} // namespace dv8
