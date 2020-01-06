#ifndef DV8_UDP_H
#define DV8_UDP_H

#include <dv8.h>

namespace dv8 {

namespace udp {
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

void InitAll(v8::Local<v8::Object> exports);

class UDP : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);
  	uv_udp_t *handle;
		uv_buf_t in;
		uv_buf_t out;
		v8::Persistent<v8::Function> onMessage;
		v8::Persistent<v8::Function> onError;
		v8::Persistent<v8::Function> onClose;
		v8::Persistent<v8::Function> onSend;

protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

	private:

		UDP() {
		}

		~UDP() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void Open(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Bind(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Send(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Start(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Stop(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetPeerName(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSockName(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetBroadcast(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void OnMessage(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void OnError(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void OnClose(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void OnSend(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}

#endif
