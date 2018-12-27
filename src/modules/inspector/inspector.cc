#include "inspector.h"

namespace dv8 {

namespace inspector {
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
	using dv8::inspector::InspectorClient;

	void Inspector::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "Inspector"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", Inspector::Setup);
	
		DV8_SET_EXPORT(isolate, tpl, "Inspector", exports);
	}

	void Inspector::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			Inspector* obj = new Inspector();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void Inspector::Setup(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		InspectorClient inspector_client(context, true);
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		Inspector* obj = ObjectWrap::Unwrap<Inspector>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
}
}	
