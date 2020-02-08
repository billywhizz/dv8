#ifndef DV8_FileSystem_H
#define DV8_FileSystem_H

#include <dv8.h>

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

void InitAll(Local<Object> exports);

class File : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);
		int fd;

protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);
	private:

		File() {
		}

		~File() {
		}

		struct iovec* in;
		struct iovec* out;

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void Open(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Read(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Write(const v8::FunctionCallbackInfo<v8::Value>& args);

};
/*
class FileSystem : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);

protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);
	private:

		FileSystem() {
		}

		~FileSystem() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void Unlink(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Mkdir(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Rmdir(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FStat(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Rename(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FSync(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FTruncate(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Copy(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SendFile(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Readdir(const v8::FunctionCallbackInfo<v8::Value>& args);

};
*/
}
}

#endif
