#include "mbedtls.h"

namespace dv8 {

namespace mbedtls {
	using dv8::builtins::Environment;
	using dv8::builtins::Buffer;

	void Crypto::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "Crypto").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_METHOD(isolate, tpl, "hash", Crypto::Hash);
		DV8_SET_METHOD(isolate, tpl, "hmac", Crypto::Hmac);

		DV8_SET_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_MD2), "MD2", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_MD4), "MD4", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_MD5), "MD5", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_SHA1), "SHA1", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_SHA256), "SHA256", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_SHA512), "SHA512", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_RIPEMD160), "RIPEMD160", tpl);

		DV8_SET_EXPORT(isolate, tpl, "Crypto", exports);
	}

	void Hash::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "Hash").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_METHOD(isolate, tpl, "md5", Hash::MD5);
		DV8_SET_METHOD(isolate, tpl, "sha256", Hash::SHA256);
	
		DV8_SET_EXPORT(isolate, tpl, "Hash", exports);
	}

	void Hmac::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "Hmac").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_METHOD(isolate, tpl, "md5", Hmac::MD5);
		DV8_SET_METHOD(isolate, tpl, "sha256", Hmac::SHA256);
	
		DV8_SET_EXPORT(isolate, tpl, "Hmac", exports);
	}

	void Crypto::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			Crypto* obj = new Crypto();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
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

	void Hmac::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			Hmac* obj = new Hmac();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void Crypto::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		#if TRACE
		fprintf(stderr, "Crypto::Destroy\n");
		#endif
	}

	void Hash::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		#if TRACE
		fprintf(stderr, "Hash::Destroy\n");
		#endif
	}

	void Hmac::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		#if TRACE
		fprintf(stderr, "Hmac::Destroy\n");
		#endif
	}

	void Crypto::Hash(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
    int i, ret;
		uint32_t algorithm = args[0]->Uint32Value(context).ToChecked();
		String::Utf8Value str(isolate, args[1]);
		v8::Local<v8::Array> answer = args[2].As<v8::Array>();
		if (algorithm == MBEDTLS_MD_MD5) {
			unsigned char digest[16];
			if( ( ret = mbedtls_md5_ret( (unsigned char *) *str, str.length(), digest ) ) != 0 ) {
				args.GetReturnValue().Set(Integer::New(isolate, ret));
				return;
			}
			for( i = 0; i < 16; i++ ) {
				answer->Set(context, i, v8::Number::New(isolate, digest[i]));
			}
		} else if (algorithm == MBEDTLS_MD_SHA256) {
			unsigned char digest[32];
			if( ( ret = mbedtls_sha256_ret( (unsigned char *) *str, str.length(), digest, 0 ) ) != 0 ) {
				args.GetReturnValue().Set(Integer::New(isolate, ret));
				return;
			}
			for( i = 0; i < 32; i++ ) {
				answer->Set(context, i, v8::Number::New(isolate, digest[i]));
			}
		}
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Crypto::Hmac(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		v8::HandleScope handleScope(isolate);
    int i, ret;
		mbedtls_md_type_t algorithm = (mbedtls_md_type_t)args[0]->Uint32Value(context).ToChecked();
		String::Utf8Value key(isolate, args[1]);
		String::Utf8Value str(isolate, args[2]);
		v8::Local<v8::Array> answer = args[3].As<v8::Array>();
		int bytes = 16;
		if (algorithm == MBEDTLS_MD_SHA256) {
			bytes = 32;
		} else if (algorithm == MBEDTLS_MD_SHA512) {
			bytes = 64;
		}
    unsigned char digest[bytes];
    if( ( ret = mbedtls_md_hmac( mbedtls_md_info_from_type(algorithm), (unsigned char *) *key, key.length(), (unsigned char *) *str, str.length(), digest ) ) != 0 ) {
			args.GetReturnValue().Set(Integer::New(isolate, ret));
			return;
		}
    for( i = 0; i < bytes; i++ ) {
      answer->Set(context, i, v8::Number::New(isolate, digest[i]));
		}
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

	void Hash::SHA256(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		String::Utf8Value str(isolate, args[0]);
    int i, ret;
    unsigned char digest[32];
		v8::Local<v8::Array> answer = args[1].As<v8::Array>();
    if( ( ret = mbedtls_sha256_ret( (unsigned char *) *str, str.length(), digest, 0 ) ) != 0 ) {
			args.GetReturnValue().Set(Integer::New(isolate, ret));
			return;
		}
    for( i = 0; i < 32; i++ ) {
      answer->Set(context, i, v8::Number::New(isolate, digest[i]));
		}
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Hmac::MD5(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		v8::HandleScope handleScope(isolate);
		String::Utf8Value key(isolate, args[0]);
		String::Utf8Value str(isolate, args[1]);
    int i, ret;
    unsigned char digest[16];
		v8::Local<v8::Array> answer = args[2].As<v8::Array>();
    if( ( ret = mbedtls_md_hmac( mbedtls_md_info_from_type(MBEDTLS_MD_MD5), (unsigned char *) *key, key.length(), (unsigned char *) *str, str.length(), digest ) ) != 0 ) {
			args.GetReturnValue().Set(Integer::New(isolate, ret));
			return;
		}
    for( i = 0; i < 16; i++ ) {
      answer->Set(context, i, v8::Number::New(isolate, digest[i]));
		}
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Hmac::SHA256(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		v8::HandleScope handleScope(isolate);
		String::Utf8Value key(isolate, args[0]);
		String::Utf8Value str(isolate, args[1]);
    int i, ret;
    unsigned char digest[32];
		v8::Local<v8::Array> answer = args[2].As<v8::Array>();
    if( ( ret = mbedtls_md_hmac( mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), (unsigned char *) *key, key.length(), (unsigned char *) *str, str.length(), digest ) ) != 0 ) {
			args.GetReturnValue().Set(Integer::New(isolate, ret));
			return;
		}
    for( i = 0; i < 32; i++ ) {
      answer->Set(context, i, v8::Number::New(isolate, digest[i]));
		}
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void InitAll(Local<Object> exports)
	{
		Crypto::Init(exports);
		Hash::Init(exports);
		Hmac::Init(exports);
	}

}
}	

extern "C" {
	void* _register_mbedtls() {
		return (void*)dv8::mbedtls::InitAll;
	}
}
