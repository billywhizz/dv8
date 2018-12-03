#ifndef DV8_FileSystem_H
#define DV8_FileSystem_H

#include <dv8.h>

namespace dv8 {

namespace fs {

class File : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);

	private:

		File() {
		}

		~File() {
		}

		uv_fs_t req;
		uv_buf_t in;
		uv_buf_t out;

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Open(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Read(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Write(const v8::FunctionCallbackInfo<v8::Value>& args);

};

class FileSystem : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);

	private:

		FileSystem() {
		}

		~FileSystem() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Unlink(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Mkdir(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FStat(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Rename(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FSync(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FTruncate(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Copy(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SendFile(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
