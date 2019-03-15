#include "posix.h"

namespace dv8 {
namespace posix {
	using v8::Local;
	using v8::Object;

	void InitAll(Local<Object> exports) {
		Queue::Init(exports);
	}
}
}

extern "C" {
	void* _register_posix() {
		return (void*)dv8::posix::InitAll;
	}
}
