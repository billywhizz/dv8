#include "httpParser.h"

namespace dv8 {
namespace httpParser {
	using v8::Local;
	using v8::Object;

	void InitAll(Local<Object> exports) {
		HTTPParser::Init(exports);
	}
}
}

extern "C" {
	void* _register_httpParser() {
		return (void*)dv8::httpParser::InitAll;
	}
}
