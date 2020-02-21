#ifndef DV8_loop_H
#define DV8_loop_H

#include <dv8.h>

namespace dv8 {

namespace loop {

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

class EventLoop : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);

	protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

	private:

		EventLoop() {
		}

		~EventLoop() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void ListHandles(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Run(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
