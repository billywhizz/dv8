#include "fs.h"

namespace dv8 {

namespace fs {
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

	void FileSystem::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "FileSystem"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "open", FileSystem::Open);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "close", FileSystem::Close);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "read", FileSystem::Read);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "unlink", FileSystem::Unlink);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "mkdir", FileSystem::Mkdir);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "write", FileSystem::Write);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "fstat", FileSystem::FStat);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "rename", FileSystem::Rename);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "fsync", FileSystem::FSync);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "ftruncate", FileSystem::FTruncate);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "copy", FileSystem::Copy);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "sendfile", FileSystem::SendFile);
	
		DV8_SET_EXPORT(isolate, tpl, "FileSystem", exports);
	}

	void FileSystem::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			FileSystem* obj = new FileSystem();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void FileSystem::Open(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::Close(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::Read(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::Unlink(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::Mkdir(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::Write(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::FStat(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::Rename(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::FSync(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::FTruncate(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::Copy(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::SendFile(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}


}
}	
