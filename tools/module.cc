#include "${name}.h"

namespace dv8 {

namespace ${name} {
	using dv8::builtins::Environment;
	using dv8::builtins::Buffer;

	void InitAll(Local<Object> exports)
	{
		// initialise all the classes in this module
		// can also do any initial work here. will only be called once when 
		// library is loaded
		${className}::Init(exports);
	}

	void ${className}::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		// create a function template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "${className}").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
		// an instance method
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "hello", ${className}::Hello);
		// a static method on the template
		DV8_SET_METHOD(isolate, tpl, "hello", ${className}::HelloStatic);
		// a static constant on the template
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, 1), "A_CONSTANT", tpl);
		// store the template on the exports object
		DV8_SET_EXPORT(isolate, tpl, "${className}", exports);
	}

	void ${className}::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			${className}* obj = new ${className}();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
			#if TRACE
			fprintf(stderr, "$(name)::${className}::Create\\n");
			#endif
		}
	}

	void ${className}::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		#if TRACE
		fprintf(stderr, "$(name)::${className}::Destroy\\n");
		#endif
	}

	void ${className}::Hello(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		// in an instance method we can get the object instance calling this method
		${className}* obj = ObjectWrap::Unwrap<${className}>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
	void ${className}::HelloStatic(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

}
}	

extern "C" {
	void* _register_${name}() {
		return (void*)dv8::${name}::InitAll;
	}
}
