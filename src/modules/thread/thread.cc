#include "thread.h"
#include "builtins.h"

namespace dv8
{

namespace thread
{
using dv8::builtins::Buffer;
using dv8::builtins::Environment;

void InitAll(Local<Object> exports)
{
	Thread::Init(exports);
}

void start_context(void *data)
{
	thread_handle *th = (thread_handle *)data;
	v8::Isolate::CreateParams create_params;
	create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	v8::Isolate *isolate = v8::Isolate::New(create_params);
	{
		isolate->SetAbortOnUncaughtExceptionCallback(dv8::ShouldAbortOnUncaughtException);
		isolate->SetFatalErrorHandler(dv8::OnFatalError);
		isolate->SetOOMErrorHandler(dv8::OOMErrorHandler);
		isolate->SetPromiseRejectCallback(dv8::PromiseRejectCallback);
    isolate->SetCaptureStackTraceForUncaughtExceptions(true, 1000, v8::StackTrace::kDetailed);
    isolate->AddGCPrologueCallback(dv8::beforeGCCallback);
    isolate->AddGCEpilogueCallback(dv8::afterGCCallback);
    isolate->AddMicrotasksCompletedCallback(dv8::microTasksCallback);
    isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kAuto);
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = dv8::CreateContext(isolate);
		v8::Context::Scope context_scope(context);
		dv8::builtins::Environment *env = new dv8::builtins::Environment();
		env->AssignToContext(context);
    env->argc = th->argc;
    env->argv = th->argv;
    v8::Local<v8::Array> arguments = v8::Array::New(isolate);
    for (int i = 0; i < env->argc; i++) {
      arguments->Set(context, i, v8::String::NewFromUtf8(isolate, env->argv[i], v8::NewStringType::kNormal, strlen(env->argv[i])).ToLocalChecked());
    }
		v8::Local<v8::Object> globalInstance = context->Global();
		v8::Local<v8::Value> obj = globalInstance->Get(context, v8::String::NewFromUtf8(isolate, "dv8", v8::NewStringType::kNormal).ToLocalChecked()).ToLocalChecked();
    v8::Local<v8::Object> dv8 = v8::Local<v8::Object>::Cast(obj);
		globalInstance->Set(context, v8::String::NewFromUtf8(isolate, "global", v8::NewStringType::kNormal).ToLocalChecked(), globalInstance);
    dv8->Set(context, v8::String::NewFromUtf8(isolate, "args", v8::NewStringType::kNormal).ToLocalChecked(), arguments);
		dv8::builtins::Buffer::Init(globalInstance);
		v8::TryCatch try_catch(isolate);
		if (th->length > 0) {
			v8::Local<v8::Function> bufferObj = Local<Function>::Cast(globalInstance->Get(context, v8::String::NewFromUtf8(isolate, "Buffer", v8::NewStringType::kNormal).ToLocalChecked()).ToLocalChecked());
			Local<Function> cons = Local<Function>::New(isolate, bufferObj);
			Local<Object> instance = cons->NewInstance(context, 0, NULL).ToLocalChecked();
			Buffer *obj = new Buffer((char*)th->data, th->length);
			obj->Wrap(instance);
			dv8->Set(context, String::NewFromUtf8(isolate, "workerData", v8::NewStringType::kNormal).ToLocalChecked(), instance);
		}
		if (th->size > 0) {
			dv8->Set(context, String::NewFromUtf8(isolate, "workerSource", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, (char*)th->source, v8::NewStringType::kNormal).ToLocalChecked());
		}
		dv8->Set(context, String::NewFromUtf8(isolate, "workerName", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, (char*)th->name, v8::NewStringType::kNormal).ToLocalChecked());
		dv8->Set(context, String::NewFromUtf8(isolate, "tid", v8::NewStringType::kNormal).ToLocalChecked(), Number::New(isolate, th->tid));
		if (th->fd > 0) {
			dv8->Set(context, String::NewFromUtf8(isolate, "fd", v8::NewStringType::kNormal).ToLocalChecked(), Number::New(isolate, th->fd));
		}
		uv_loop_t *loop = (uv_loop_t *)malloc(sizeof(uv_loop_t));
		env->loop = loop;
		uv_loop_init(loop);
    v8::Local<v8::String> base = v8::String::NewFromUtf8(isolate, src_main_js, v8::NewStringType::kNormal, static_cast<int>(src_main_js_len)).ToLocalChecked();
		v8::ScriptOrigin baseorigin(v8::String::NewFromUtf8(isolate, "main.js", v8::NewStringType::kNormal).ToLocalChecked(), 
			v8::Integer::New(isolate, 0), 
			v8::Integer::New(isolate, 0), 
			v8::False(isolate), 
			v8::Local<v8::Integer>(), 
			v8::Local<v8::Value>(), 
			v8::False(isolate), 
			v8::False(isolate), 
			v8::True(isolate));
		v8::Local<v8::Module> module;
		v8::ScriptCompiler::Source basescript(base, baseorigin);
		if (!v8::ScriptCompiler::CompileModule(isolate, &basescript).ToLocal(&module)) {
      dv8::PrintStackTrace(isolate, try_catch);
			return;
		}
		v8::Maybe<bool> ok = module->InstantiateModule(context, dv8::OnModuleInstantiate);
		if (!ok.ToChecked()) {
      dv8::PrintStackTrace(isolate, try_catch);
			return;
		}
    v8::MaybeLocal<v8::Value> result = module->Evaluate(context);
		if (result.IsEmpty()) {
			if (try_catch.HasCaught()) {
				dv8::PrintStackTrace(isolate, try_catch);
				return;
			}
		}
	}
	isolate->Dispose();
	delete create_params.array_buffer_allocator;
	uv_async_send(th->async);
}

void on_thread_close(uv_handle_t *handle)
{
		//free(handle);
}

void on_context_complete(uv_async_t *async)
{
	thread_handle *th = (thread_handle *)async->data;
	uv_handle_t* handle = (uv_handle_t*)async;
	Thread *t = (Thread *)th->object;
	Isolate *isolate = Isolate::GetCurrent();
	v8::HandleScope handleScope(isolate);
	Local<Context> context = isolate->GetCurrentContext();
	Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
	Local<Function> foo = Local<Function>::New(isolate, t->onComplete);
	Local<Value> argv[1] = { v8::Null(isolate) };
	v8::TryCatch try_catch(isolate);
	foo->Call(context, context->Global(), 1, argv);
	free(th);
	if (try_catch.HasCaught()) {
		dv8::ReportException(isolate, &try_catch);
	}
	uv_close(handle, on_thread_close);
}

void Thread::Init(Local<Object> exports)
{
	Isolate *isolate = exports->GetIsolate();
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

	tpl->SetClassName(String::NewFromUtf8(isolate, "Thread").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "start", Thread::Start);
	DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stop", Thread::Stop);

	DV8_SET_EXPORT(isolate, tpl, "Thread", exports);
}

void Thread::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
  Isolate *isolate = data.GetIsolate();
  v8::HandleScope handleScope(isolate);
  ObjectWrap *wrap = data.GetParameter();
  Thread* thread = static_cast<Thread *>(wrap);
		#if TRACE
		fprintf(stderr, "Thread::Destroy\n");
		#endif
}

void Thread::New(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	HandleScope handle_scope(isolate);
	if (args.IsConstructCall()) {
		Thread *obj = new Thread();
		obj->Wrap(args.This());
		args.GetReturnValue().Set(args.This());
	}
}

void Thread::Stop(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
	v8::HandleScope handleScope(isolate);
	//TODO: need to signal the thread. then it should invoke an onExit event and shutdown event loop when it returns
	// i.e. same behaviour as when thread gets SIGTERM
	Thread *obj = ObjectWrap::Unwrap<Thread>(args.Holder());
}

void Thread::Start(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
	v8::HandleScope handleScope(isolate);
	int argc = args.Length();
	Thread *obj = ObjectWrap::Unwrap<Thread>(args.Holder());
	Local<Function> threadFunc = Local<Function>::Cast(args[0]);
	String::Utf8Value function_name(isolate, threadFunc->GetName());
	Local<Function> onComplete = Local<Function>::Cast(args[1]);
	Local<String> sourceString = threadFunc->ToString(context).ToLocalChecked();
	String::Utf8Value source(isolate, sourceString);
	obj->onComplete.Reset(isolate, onComplete);
	thread_handle *th = (thread_handle *)malloc(sizeof(thread_handle));
	obj->handle = th;
	th->length = 0;
	if (argc > 2) {
		th->fd = args[2]->Uint32Value(context).ToChecked();
	}
	if (argc > 3) {
		Buffer *b = ObjectWrap::Unwrap<Buffer>(args[3].As<v8::Object>());
		size_t len = b->_length;
		th->data = b->_data;
		th->length = b->_length;
	}
	th->name = *function_name;
	th->object = (void *)obj;
	int len = strlen(*source);
	th->size = len;
	th->source = (char*)calloc(th->size, 1);
	th->async = (uv_async_t*)calloc(1, sizeof(uv_async_t));
	th->async->data = th;
	th->argc = env->argc;
	th->argv = uv_setup_args(env->argc, env->argv);
	uv_async_init(env->loop, th->async, on_context_complete);
	strncpy(th->source, *source, len + 1);
	int r = uv_thread_create(&th->tid, start_context, (void*)th);
	args.GetReturnValue().Set(Integer::New(isolate, r));
}

} // namespace thread
} // namespace dv8

extern "C" {
	void* _register_thread() {
		return (void*)dv8::thread::InitAll;
	}
}
