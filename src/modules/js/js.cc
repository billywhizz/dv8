#include "js.h"

namespace dv8
{

namespace js
{
using dv8::builtins::Environment;
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

Persistent<Function> JS::constructor;

void JS::Init(Local<Object> exports)
{
	Isolate *isolate = exports->GetIsolate();
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

	tpl->SetClassName(String::NewFromUtf8(isolate, "JS"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "hello", JS::Hello);

	constructor.Reset(isolate, tpl->GetFunction());
	DV8_SET_EXPORT(isolate, tpl, "JS", exports);
}

void JS::New(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	HandleScope handle_scope(isolate);
	if (args.IsConstructCall())
	{
		JS *obj = new JS();
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

void JS::NewInstance(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	const unsigned argc = 2;
	Local<Value> argv[argc] = {args[0], args[1]};
	Local<Function> cons = Local<Function>::New(isolate, constructor);
	Local<Context> context = isolate->GetCurrentContext();
	Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
	args.GetReturnValue().Set(instance);
}

void JS::Hello(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(32));
	v8::HandleScope handleScope(isolate);
	JS *obj = ObjectWrap::Unwrap<JS>(args.Holder());
	args.GetReturnValue().Set(Integer::New(isolate, 0));
}

} // namespace js
} // namespace dv8
