#ifndef DV8_FileSystem_H
#define DV8_FileSystem_H

#include <dv8.h>

namespace dv8 {

namespace fs {

class FileSystem : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);

	private:

		FileSystem() {
		}

		~FileSystem() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Hello(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
