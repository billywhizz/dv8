#include "event.h"

	namespace dv8 {
	namespace event {
	  using v8::Local;
	  using v8::Object;
	
	  void InitAll(Local<Object> exports) {
		Event::Init(exports);
	  }
	}
	}
	
	extern "C" {
	  void* _register_event() {
		return (void*)dv8::event::InitAll;
	  }
	}
