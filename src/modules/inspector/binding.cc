#include "inspector.h"

namespace dv8 {
namespace inspector {
	using v8::Local;
	using v8::Object;

	void InitAll(Local<Object> exports) {
		Inspector::Init(exports);
	}
}
}

extern "C" {
	void* _register_inspector() {
		return (void*)dv8::inspector::InitAll;
	}
}
