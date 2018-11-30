#ifndef DV8_UDP_H
#define DV8_UDP_H

#include <dv8.h>

namespace dv8 {

namespace udp {

class UDP : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);

	private:

		UDP() {
		}

		~UDP() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Hello(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
