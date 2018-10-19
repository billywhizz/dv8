#include "js.h"

namespace dv8 {
namespace js {
	using v8::Local;
	using v8::Object;

	void InitAll(Local<Object> exports) {
		JS::Init(exports);
	}
}
}

extern "C" {
	void* _register_js() {
		return (void*)dv8::js::InitAll;
	}
}
