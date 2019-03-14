#include "picoHttpParser.h"

namespace dv8 {
namespace picoHttpParser {
	using v8::Local;
	using v8::Object;

	void InitAll(Local<Object> exports) {
		PicoHTTPParser::Init(exports);
	}
}
}

extern "C" {
	void* _register_picoHttpParser() {
		return (void*)dv8::picoHttpParser::InitAll;
	}
}
