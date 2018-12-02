#ifndef DV8_UDP_H
#define DV8_UDP_H

#include <dv8.h>

namespace dv8 {

namespace udp {

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

		static void OnMessage(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void OnError(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void OnClose(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void OnSend(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
