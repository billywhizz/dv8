#include "mbedtls.h"

namespace dv8 {

namespace mbedtls {
	using dv8::builtins::Environment;
	using dv8::builtins::Buffer;

	void InitAll(Local<Object> exports)
	{
		Hash::Init(exports);
	}

	void Hash::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "Hash").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "hello", Hash::Hello);
		DV8_SET_METHOD(isolate, tpl, "md5", Hash::MD5);
	
		DV8_SET_EXPORT(isolate, tpl, "Hash", exports);
	}

	void Hash::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			Hash* obj = new Hash();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void Hash::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		#if TRACE
		fprintf(stderr, "Hash::Destroy\n");
		#endif
	}

	void Hash::Hello(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Hash* obj = ObjectWrap::Unwrap<Hash>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
	void Hash::MD5(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		String::Utf8Value str(isolate, args[0]);
    int i, ret;
    unsigned char digest[16];
		v8::Local<v8::Array> answer = args[1].As<v8::Array>();
    if( ( ret = mbedtls_md5_ret( (unsigned char *) *str, str.length(), digest ) ) != 0 ) {
			args.GetReturnValue().Set(Integer::New(isolate, ret));
			return;
		}
    for( i = 0; i < 16; i++ ) {
      answer->Set(context, i, v8::Number::New(isolate, digest[i]));
		}
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

}
}	

extern "C" {
	void* _register_mbedtls() {
		return (void*)dv8::mbedtls::InitAll;
	}
}
