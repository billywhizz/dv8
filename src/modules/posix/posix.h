#ifndef DV8_Queue_H
#define DV8_Queue_H

#include <dv8.h>
#include <mqueue.h>

namespace dv8 {

namespace posix {

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
using dv8::builtins::Environment;
using dv8::builtins::Buffer;

enum socket_type
{
  CONSUMER = 0,
  PRODUCER
};

class Queue : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);
    mqd_t mq;
    struct mq_attr attr;
		char* data;
		const char* sockName;
		
	private:

		Queue() {
		}

		~Queue() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Open(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Send(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Recv(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Unlink(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
