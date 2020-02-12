#include "memory.h"

namespace dv8 {

namespace memory {
	using dv8::builtins::Environment;
	using dv8::builtins::Buffer;

	void InitAll(Local<Object> exports)
	{
		// initialise all the classes in this module
		// can also do any initial work here. will only be called once when 
		// library is loaded
		Memory::Init(exports);
	}

	void Memory::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		// create a function template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "Memory").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
		DV8_SET_METHOD(isolate, tpl, "readString", Memory::ReadString);
		DV8_SET_METHOD(isolate, tpl, "readUint32", Memory::ReadUint32);
		DV8_SET_METHOD(isolate, tpl, "readUint64", Memory::ReadUint64);
		DV8_SET_METHOD(isolate, tpl, "readAddress", Memory::ReadAddress);
		DV8_SET_METHOD(isolate, tpl, "readInt32", Memory::ReadInt32);
		DV8_SET_METHOD(isolate, tpl, "readInt64", Memory::ReadInt64);
		DV8_SET_EXPORT(isolate, tpl, "Memory", exports);
	}

	void Memory::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			Memory* obj = new Memory();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
			#if TRACE
			fprintf(stderr, "$(name)::Memory::Create\n");
			#endif
		}
	}

	void Memory::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		#if TRACE
		fprintf(stderr, "$(name)::Memory::Destroy\n");
		#endif
	}

	void Memory::ReadString(const v8::FunctionCallbackInfo<v8::Value>& args) {
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Local<BigInt> address64 = Local<BigInt>::Cast(args[0]);
		std::uintptr_t address = address64->Uint64Value();
		Local<BigInt> len64 = Local<BigInt>::Cast(args[1]);
		const unsigned long len = len64->Uint64Value();
		const char* str = reinterpret_cast<const char*>(address);
		Local<String> result = v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal, static_cast<int>(len)).ToLocalChecked();
		args.GetReturnValue().Set(result);
	}

	void Memory::ReadUint32(const v8::FunctionCallbackInfo<v8::Value>& args) {
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Local<BigInt> address64 = Local<BigInt>::Cast(args[0]);
		std::uintptr_t address = address64->Uint64Value();
		uint32_t* val = reinterpret_cast<uint32_t*>(address);
		Local<Integer> bigVal = Uint32::NewFromUnsigned(isolate, *val);
		args.GetReturnValue().Set(bigVal);
	}
	
	void Memory::ReadUint64(const v8::FunctionCallbackInfo<v8::Value>& args) {
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Local<BigInt> address64 = Local<BigInt>::Cast(args[0]);
		std::uintptr_t address = address64->Uint64Value();
		uint64_t* val = reinterpret_cast<uint64_t*>(address);
		Local<BigInt> bigVal = BigInt::NewFromUnsigned(isolate, *val);
		args.GetReturnValue().Set(bigVal);
	}

	void Memory::ReadAddress(const v8::FunctionCallbackInfo<v8::Value>& args) {
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Local<BigInt> address64 = Local<BigInt>::Cast(args[0]);
		std::uintptr_t address = address64->Uint64Value();
		uint64_t* val = reinterpret_cast<uint64_t*>(address);
		Local<BigInt> bigVal = BigInt::NewFromUnsigned(isolate, *val);
		args.GetReturnValue().Set(bigVal);
	}

	void Memory::ReadInt32(const v8::FunctionCallbackInfo<v8::Value>& args) {
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Local<BigInt> address64 = Local<BigInt>::Cast(args[0]);
		std::uintptr_t address = address64->Uint64Value();
		int32_t* val = reinterpret_cast<int32_t*>(address);
		Local<Integer> bigVal = Int32::NewFromUnsigned(isolate, *val);
		args.GetReturnValue().Set(bigVal);
	}

	void Memory::ReadInt64(const v8::FunctionCallbackInfo<v8::Value>& args) {
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Local<BigInt> address64 = Local<BigInt>::Cast(args[0]);
		std::uintptr_t address = address64->Uint64Value();
		int64_t* val = reinterpret_cast<int64_t*>(address);
		Local<BigInt> bigVal = BigInt::NewFromUnsigned(isolate, *val);
		args.GetReturnValue().Set(bigVal);
	}

}
}	

extern "C" {
	void* _register_memory() {
		return (void*)dv8::memory::InitAll;
	}
}
