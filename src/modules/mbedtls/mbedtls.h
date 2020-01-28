#ifndef DV8_Hash_H
#define DV8_Hash_H

#include <dv8.h>
#include <mbedtls/md2.h>
#include <mbedtls/md4.h>
#include <mbedtls/md5.h>
#include <mbedtls/ripemd160.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#include <mbedtls/sha512.h>
#include <mbedtls/md.h>

namespace dv8 {

namespace mbedtls {

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

class Crypto : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);

	protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

	private:

		Crypto() {
		}

		~Crypto() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Hash(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Hmac(const v8::FunctionCallbackInfo<v8::Value>& args);

};

class Hash : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);

	protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

	private:

		Hash() {
		}

		~Hash() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void MD5(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SHA256(const v8::FunctionCallbackInfo<v8::Value>& args);

};

class Hmac : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);

	protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

	private:

		Hmac() {
		}

		~Hmac() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void MD5(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SHA256(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
