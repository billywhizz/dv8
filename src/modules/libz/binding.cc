#include "libz.h"

namespace dv8 {
namespace libz {
	using v8::Local;
	using v8::Object;

	void InitAll(Local<Object> exports) {
		ZLib::Init(exports);
	}
}
}

extern "C" {
	void* _register_libz() {
		return (void*)dv8::libz::InitAll;
	}
}
