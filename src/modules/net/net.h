#ifndef DV8_net_H
#define DV8_net_H

#include <dv8.h>
#include <jsys_http.h>

namespace dv8 {

namespace net {

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
using v8::BigInt;

typedef struct
{
  uint8_t onRequest;
} http_callbacks_t;

typedef struct
{
  uint8_t onConnect;
  uint8_t onData;
  uint8_t onEnd;
} socket_callbacks_t;

void InitAll(Local<Object> exports);

class Http : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);
		v8::Persistent<v8::Function> onRequest;
		http_callbacks_t callbacks;

	protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

	private:

		Http() {
		}

		~Http() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Listen(const v8::FunctionCallbackInfo<v8::Value>& args);
	  static void OnRequest(const v8::FunctionCallbackInfo<v8::Value> &args);

};

class Socket : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);
		v8::Persistent<v8::Function> onConnect;
		v8::Persistent<v8::Function> onData;
		v8::Persistent<v8::Function> onEnd;
		socket_callbacks_t callbacks;
		jsys_descriptor* handle;
		int pairfd;

	protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

	private:

		Socket() {
		}

		~Socket() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void Pair(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Open(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Bind(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Listen(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Connect(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Write(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Pause(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Resume(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);

	  static void OnConnect(const v8::FunctionCallbackInfo<v8::Value> &args);
	  static void OnData(const v8::FunctionCallbackInfo<v8::Value> &args);
	  static void OnEnd(const v8::FunctionCallbackInfo<v8::Value> &args);

};

}
}
#endif
