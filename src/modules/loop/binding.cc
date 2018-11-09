#include "loop.h"

namespace dv8 {
namespace loop {
	using v8::Local;
	using v8::Object;

	void InitAll(Local<Object> exports) {
		EventLoop::Init(exports);
	}
}
}

extern "C" {
	void* _register_loop() {
		return (void*)dv8::loop::InitAll;
	}
}
