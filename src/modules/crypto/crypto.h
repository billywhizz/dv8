#ifndef DV8_MBEDTLS_Hash_H
#define DV8_MBEDTLS_Hash_H

#include <dv8.h>
#include <mbedtls/md4.h>
#include <mbedtls/md5.h>
#include <mbedtls/ripemd160.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#include <mbedtls/sha512.h>
#include <mbedtls/md.h>
#include <mbedtls/version.h>

namespace dv8 {

namespace crypto {

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

void InitAll(Local<Object> exports);

class Hash : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);

	protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

	private:
		struct iovec in;
		struct iovec out;
		const mbedtls_md_info_t* algorithm;

		Hash() {
		}

		~Hash() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Digest(const v8::FunctionCallbackInfo<v8::Value>& args);

};

class Hmac : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);

	protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

	private:
		struct iovec in;
		struct iovec out;
		struct iovec key;
		const mbedtls_md_info_t* algorithm;

		Hmac() {
		}

		~Hmac() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Digest(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
