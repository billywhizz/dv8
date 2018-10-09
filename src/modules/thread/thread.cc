#include "thread.h"

namespace dv8
{

namespace thread
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
using dv8::builtins::Buffer;

Persistent<Function> Thread::constructor;

void spawn_context(uv_work_t *req) {
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate *isolate = v8::Isolate::New(create_params);
    {
      isolate->SetAbortOnUncaughtExceptionCallback(dv8::ShouldAbortOnUncaughtException);
      isolate->SetFatalErrorHandler(dv8::OnFatalError);
      v8::Isolate::Scope isolate_scope(isolate);
      v8::HandleScope handle_scope(isolate);
      v8::Local<v8::Context> context = dv8::CreateContext(isolate);
      if (context.IsEmpty()) {
        fprintf(stderr, "Error creating context\n");
        return;
      }
      const char *str = "./foo.js";
      v8::Context::Scope context_scope(context);
      v8::Local<v8::Object> globalInstance = context->Global();
      globalInstance->Set(v8::String::NewFromUtf8(isolate, "global", v8::NewStringType::kNormal).ToLocalChecked(), globalInstance);
      dv8::builtins::Buffer::Init(globalInstance);
      v8::TryCatch try_catch(isolate);
      v8::MaybeLocal<v8::String> source = dv8::ReadFile(isolate, str);
      if (try_catch.HasCaught()) {
        dv8::DecorateErrorStack(isolate, try_catch);
        return;
      }
      v8::MaybeLocal<v8::Script> script = v8::Script::Compile(context, source.ToLocalChecked());
      if (try_catch.HasCaught()) {
        dv8::DecorateErrorStack(isolate, try_catch);
        return;
      }
      script.ToLocalChecked()->Run(context);
      if (try_catch.HasCaught()) {
        dv8::DecorateErrorStack(isolate, try_catch);
        return;
      }
      int alive;
      do {
        uv_run(uv_default_loop(), UV_RUN_DEFAULT);
        alive = uv_loop_alive(uv_default_loop());
        if (alive != 0) {
          continue;
        }
        alive = uv_loop_alive(uv_default_loop());
      } while (alive != 0);
      dv8::shutdown();
      uv_tty_reset_mode();
      int r = uv_loop_close(uv_default_loop());
      if (r != 0) {
        fprintf(stderr, "uv_loop_close: %i\n", r);
      }
    }
    //isolate->Exit();
    isolate->Dispose();
    delete create_params.array_buffer_allocator;
}

void thread_main(uv_work_t *req) {
	uint8_t i = 10;
	thread_handle* th = (thread_handle*)req->data;
	uint8_t* b = (uint8_t*)th->data;
	while (--i > 0) {
		*b = i;
		b++;
		sleep(1);
	}
}

void after_thread(uv_work_t *req, int status) {
    fprintf(stderr, "Done on the thread\n");
	thread_handle* th = (thread_handle*)req->data;
    Thread* t = (Thread*)th->object;
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    const unsigned int argc = 0;
    Local<Value> argv[argc] = { };
    Local<Function> foo = Local<Function>::New(isolate, t->onComplete);
    v8::TryCatch try_catch(isolate);
    foo->Call(isolate->GetCurrentContext()->Global(), 0, argv);
    if (try_catch.HasCaught()) {
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
	DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "createContext", Thread::CreateContext);

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
	Buffer* b = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
	Local<Function> onComplete = Local<Function>::Cast(args[1]);
	obj->onComplete.Reset(isolate, onComplete);
	size_t len = b->_length;
	obj->handle = (uv_work_t*)malloc(sizeof(uv_work_t));
	thread_handle* th = (thread_handle*)malloc(sizeof(thread_handle));
	th->data = b->_data;
	th->length = b->_length;
	th->object = (void*)obj;
	obj->handle->data = (void*)th;
	uv_queue_work(uv_default_loop(), obj->handle, thread_main, after_thread);
	args.GetReturnValue().Set(Integer::New(isolate, 0));
}

void Thread::CreateContext(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	v8::HandleScope handleScope(isolate);
	Thread *obj = ObjectWrap::Unwrap<Thread>(args.Holder());
	Buffer* b = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
	Local<Function> onComplete = Local<Function>::Cast(args[1]);
	obj->onComplete.Reset(isolate, onComplete);
	size_t len = b->_length;
	obj->handle = (uv_work_t*)malloc(sizeof(uv_work_t));
	thread_handle* th = (thread_handle*)malloc(sizeof(thread_handle));
	th->data = b->_data;
	th->length = b->_length;
	th->object = (void*)obj;
	obj->handle->data = (void*)th;
	uv_queue_work(uv_default_loop(), obj->handle, spawn_context, after_thread);
	args.GetReturnValue().Set(Integer::New(isolate, 0));
}

} // namespace thread
} // namespace dv8
