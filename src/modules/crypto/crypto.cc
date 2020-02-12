#include "crypto.h"

namespace dv8 {

namespace crypto {
	using dv8::builtins::Environment;
	using dv8::builtins::Buffer;

	void InitAll(Local<Object> exports)
	{
		Isolate* isolate = exports->GetIsolate();
		char version_string[256];
		mbedtls_version_get_string_full(version_string);
		DV8_SET_EXPORT_CONSTANT(isolate, String::NewFromUtf8(isolate, version_string).ToLocalChecked(), "version", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_NONE), "NONE", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_MD2), "MD2", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_MD4), "MD4", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_MD5), "MD5", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_SHA1), "SHA1", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_SHA224), "SHA224", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_SHA256), "SHA256", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_SHA384), "SHA384", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_SHA512), "SHA512", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, MBEDTLS_MD_RIPEMD160), "RIPEMD160", exports);
		Hash::Init(exports);
		Hmac::Init(exports);
	}

	void Hash::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "Hash").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", Hash::Setup);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "digest", Hash::Digest);

		DV8_SET_EXPORT(isolate, tpl, "Hash", exports);
	}

	void Hmac::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "Hmac").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", Hmac::Setup);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "digest", Hmac::Digest);

		DV8_SET_EXPORT(isolate, tpl, "Hmac", exports);
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

	void Hash::Setup(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		Hash *obj = ObjectWrap::Unwrap<Hash>(args.Holder());
		Local<Context> context = isolate->GetCurrentContext();
		obj->algorithm = mbedtls_md_info_from_type((mbedtls_md_type_t)args[0]->Uint32Value(context).ToChecked());
		Buffer *b = ObjectWrap::Unwrap<Buffer>(args[1].As<v8::Object>());
		obj->in.iov_base = (char*)b->_data;
		obj->in.iov_len = b->_length;
		b = ObjectWrap::Unwrap<Buffer>(args[2].As<v8::Object>());
		obj->out.iov_base = (char*)b->_data;
		obj->out.iov_len = b->_length;
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Hash::Digest(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		Hash *obj = ObjectWrap::Unwrap<Hash>(args.Holder());
		Local<Context> context = isolate->GetCurrentContext();
		uint32_t len = obj->in.iov_len;
		if (args.Length() > 0) {
			len = args[0]->Uint32Value(context).ToChecked();
		}
		unsigned char* source = (unsigned char*)obj->in.iov_base;
		unsigned char* digest = (unsigned char*)obj->out.iov_base;
		int ret = mbedtls_md( obj->algorithm, source, len, digest );
		args.GetReturnValue().Set(Integer::New(isolate, ret));
	}

	void Hmac::Setup(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		Hmac *obj = ObjectWrap::Unwrap<Hmac>(args.Holder());
		Local<Context> context = isolate->GetCurrentContext();
		obj->algorithm = mbedtls_md_info_from_type((mbedtls_md_type_t)args[0]->Uint32Value(context).ToChecked());
		Buffer *b = ObjectWrap::Unwrap<Buffer>(args[1].As<v8::Object>());
		obj->in.iov_base = (char*)b->_data;
		obj->in.iov_len = b->_length;
		b = ObjectWrap::Unwrap<Buffer>(args[2].As<v8::Object>());
		obj->out.iov_base = (char*)b->_data;
		obj->out.iov_len = b->_length;
		String::Utf8Value key(isolate, args[3]);
		obj->key.iov_base = *key;
		obj->key.iov_len = key.length();
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Hmac::Digest(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		Hmac *obj = ObjectWrap::Unwrap<Hmac>(args.Holder());
		Local<Context> context = isolate->GetCurrentContext();
		uint32_t len = obj->in.iov_len;
		if (args.Length() > 0) {
			len = args[0]->Uint32Value(context).ToChecked();
		}
		unsigned char* source = (unsigned char*)obj->in.iov_base;
		unsigned char* digest = (unsigned char*)obj->out.iov_base;
		unsigned char* key = (unsigned char*)obj->key.iov_base;
		int keylen = obj->key.iov_len;
		int ret = mbedtls_md_hmac( obj->algorithm, key, keylen, source, len, digest );
		args.GetReturnValue().Set(Integer::New(isolate, ret));
	}

}
}	

extern "C" {
	void* _register_crypto() {
		return (void*)dv8::crypto::InitAll;
	}
}
