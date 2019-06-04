#include "thread.h"
#include "builtins.h"

namespace dv8
{

namespace thread
{
using dv8::builtins::Buffer;
using dv8::builtins::Environment;
using v8::Array;
using v8::ArrayBufferCreationMode;
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

void start_context(uv_work_t *req)
{
	thread_handle *th = (thread_handle *)req->data;

	v8::Isolate::CreateParams create_params;
	create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	v8::Isolate *isolate = v8::Isolate::New(create_params);
	{
		isolate->SetAbortOnUncaughtExceptionCallback(dv8::ShouldAbortOnUncaughtException);
		isolate->SetFatalErrorHandler(dv8::OnFatalError);
		isolate->SetOOMErrorHandler(dv8::OOMErrorHandler);
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope handle_scope(isolate);
		isolate->SetPromiseRejectCallback(dv8::PromiseRejectCallback);
		Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
		global->Set(String::NewFromUtf8(isolate, "version", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Version));
		global->Set(String::NewFromUtf8(isolate, "print", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, Print));
		global->Set(String::NewFromUtf8(isolate, "library", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, LoadModule));
		global->Set(String::NewFromUtf8(isolate, "gc", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, CollectGarbage));
		global->Set(String::NewFromUtf8(isolate, "env", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, EnvVars));
		global->Set(String::NewFromUtf8(isolate, "onExit", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, OnExit));
		global->Set(String::NewFromUtf8(isolate, "memoryUsage", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, MemoryUsage));
		global->Set(String::NewFromUtf8(isolate, "onUnhandledRejection", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, OnUnhandledRejection));
		v8::Local<v8::Context> context = Context::New(isolate, NULL, global);
		dv8::builtins::Environment *env = new dv8::builtins::Environment();
		env->error = &th->error;
		env->AssignToContext(context);
		env->err.Reset();
		v8::Local<v8::Object> globalInstance = context->Global();
		v8::Context::Scope context_scope(context);
		dv8::builtins::Buffer::Init(globalInstance);
		globalInstance->Set(v8::String::NewFromUtf8(isolate, "global", v8::NewStringType::kNormal).ToLocalChecked(), globalInstance);
		if (th->length > 0) {
			v8::Local<v8::Function> bufferObj = Local<Function>::Cast(globalInstance->Get(v8::String::NewFromUtf8(isolate, "Buffer", v8::NewStringType::kNormal).ToLocalChecked()));
			Local<Function> cons = Local<Function>::New(isolate, bufferObj);
			Local<Object> instance = cons->NewInstance(context, 0, NULL).ToLocalChecked();
			Buffer *obj = new Buffer((char*)th->data, th->length);
			obj->Wrap(instance);
			globalInstance->Set(String::NewFromUtf8(isolate, "workerData", v8::NewStringType::kNormal).ToLocalChecked(), instance);
		}
		uv_loop_t *loop = (uv_loop_t *)malloc(sizeof(uv_loop_t));
		env->loop = loop;
		uv_loop_init(loop);

		v8::MaybeLocal<v8::String> base = v8::String::NewFromUtf8(isolate, src_main_js, v8::NewStringType::kNormal, static_cast<int>(src_main_js_len));
		v8::ScriptOrigin baseorigin(v8::String::NewFromUtf8(isolate, th->name, v8::NewStringType::kNormal).ToLocalChecked(), 
			v8::Integer::New(isolate, 0), 
			v8::Integer::New(isolate, 0), 
			v8::False(isolate), 
			v8::Local<v8::Integer>(), 
			v8::Local<v8::Value>(), 
			v8::False(isolate), 
			v8::False(isolate), 
			v8::True(isolate));
		v8::Local<v8::Module> module;
		v8::TryCatch try_catch(isolate);
		v8::ScriptCompiler::Source basescript(base.ToLocalChecked(), baseorigin);
		if (!v8::ScriptCompiler::CompileModule(isolate, &basescript).ToLocal(&module)) {
			dv8::ReportException(isolate, &try_catch);
			return;
		}
		v8::Maybe<bool> ok = module->InstantiateModule(context, dv8::OnModuleInstantiate);
		if (!ok.ToChecked()) {
			dv8::ReportException(isolate, &try_catch);
			return;
		}
		globalInstance->Set(String::NewFromUtf8(isolate, "workerSource", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, (char*)th->source, v8::NewStringType::kNormal).ToLocalChecked());
		module->Evaluate(context);
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
    Local<Context> context = isolate->GetCurrentContext();
    Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(32));
	Local<Function> foo = Local<Function>::New(isolate, t->onComplete);
	Local<Value> errObj;
	if (env->err.IsEmpty()) {
		errObj = Null(isolate);
	} else {
		errObj = Local<Object>::New(isolate, env->err);
	}
	js_error* jsError = &th->error;
	if (jsError->hasError == 1) {
		Local<Object> o = Object::New(isolate);
		o->Set(String::NewFromUtf8(isolate, "line", v8::NewStringType::kNormal).ToLocalChecked(), Integer::New(isolate, jsError->linenum));
		if (jsError->filename) {
			o->Set(String::NewFromUtf8(isolate, "filename", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, jsError->filename, v8::NewStringType::kNormal).ToLocalChecked());
			free(jsError->filename);
		}
		if (jsError->exception) {
			o->Set(String::NewFromUtf8(isolate, "exception", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, jsError->exception, v8::NewStringType::kNormal).ToLocalChecked());
			free(jsError->exception);
		}
		if (jsError->sourceline) {
			o->Set(String::NewFromUtf8(isolate, "sourceline", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, jsError->sourceline, v8::NewStringType::kNormal).ToLocalChecked());
			free(jsError->sourceline);
		}
		if (jsError->stack) {
			//o->Set(String::NewFromUtf8(isolate, "stack", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, jsError->stack, v8::NewStringType::kNormal).ToLocalChecked());
			//free(jsError->stack);
		}
		Local<Value> argv[2] = { o, Integer::New(isolate, status) };
		v8::TryCatch try_catch(isolate);
		foo->Call(isolate->GetCurrentContext()->Global(), 2, argv);
		if (try_catch.HasCaught()) {
			dv8::ReportException(isolate, &try_catch);
		}
	} else {
		Local<Value> argv[2] = { v8::Null(isolate), Integer::New(isolate, status) };
		v8::TryCatch try_catch(isolate);
		foo->Call(isolate->GetCurrentContext()->Global(), 2, argv);
		if (try_catch.HasCaught()) {
			dv8::ReportException(isolate, &try_catch);
		}
	}
}

void Thread::Init(Local<Object> exports)
{
	Isolate *isolate = exports->GetIsolate();
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

	tpl->SetClassName(String::NewFromUtf8(isolate, "Thread"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "start", Thread::Start);
	DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stop", Thread::Stop);

	DV8_SET_EXPORT(isolate, tpl, "Thread", exports);
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
	Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(32));
	v8::HandleScope handleScope(isolate);
	//TODO: need to signal the thread. then it should invoke an onExit event and shutdown event loop when it returns
	// i.e. same behaviour as when thread gets SIGTERM
	Thread *obj = ObjectWrap::Unwrap<Thread>(args.Holder());
}

void Thread::Start(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(32));
	v8::HandleScope handleScope(isolate);
	int argc = args.Length();
	Thread *obj = ObjectWrap::Unwrap<Thread>(args.Holder());
	Local<Function> threadFunc = Local<Function>::Cast(args[0]);
	String::Utf8Value function_name(args.GetIsolate(), threadFunc->GetName());
	Local<Function> onComplete = Local<Function>::Cast(args[1]);
	Local<String> sourceString = threadFunc->ToString(isolate);
	String::Utf8Value source(isolate, sourceString);
	obj->onComplete.Reset(isolate, onComplete);
	obj->handle = (uv_work_t *)malloc(sizeof(uv_work_t));
	thread_handle *th = (thread_handle *)malloc(sizeof(thread_handle));
	if (argc > 2) {
		Buffer *b = ObjectWrap::Unwrap<Buffer>(args[2].As<v8::Object>());
		size_t len = b->_length;
		th->data = b->_data;
		th->length = b->_length;
	}
	else {
		th->length = 0;
	}
	th->name = *function_name;
	th->error.hasError = 0;
	th->object = (void *)obj;
	int len = strlen(*source);
	int first = 0;
	int last = 0;
	int off = 0;
	char* ss = *source;
	while(len--) {
		char* c = ss + off;
		if (c[0] == '{') {
			if (first == 0) first = off + 1;
		} else if (c[0] == '}') {
			last = off;
		}
		off++;
	}
	th->size = ((last - first));
	th->source = (char*)calloc(th->size, 1);
	char* start = *source;
	start += first;
	memcpy(th->source, (void*)start, th->size);
	obj->handle->data = (void *)th;
	uv_queue_work(env->loop, obj->handle, start_context, on_context_complete);
	args.GetReturnValue().Set(Integer::New(isolate, 0));
}

} // namespace thread
} // namespace dv8
