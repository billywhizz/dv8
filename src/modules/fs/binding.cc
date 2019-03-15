#include "fs.h"

namespace dv8 {
namespace fs {
	using v8::Local;
	using v8::Object;

	void InitAll(Local<Object> exports) {
		FileSystem::Init(exports);
		File::Init(exports);
	}
}
}

extern "C" {
	void* _register_fs() {
		return (void*)dv8::fs::InitAll;
	}
}
