#ifndef DV8_Memory_H
#define DV8_Memory_H

#include <dv8.h>

namespace dv8 {

namespace memory {

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
using v8::Uint32;
using v8::Int32;

void InitAll(Local<Object> exports);

class Memory : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);

	protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

	private:

		Memory() {
		}

		~Memory() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void ReadString(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void ReadUint32(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void ReadUint64(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void ReadAddress(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void ReadInt32(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void ReadInt64(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
