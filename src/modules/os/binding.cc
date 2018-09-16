#include "os.h"

namespace dv8 {
namespace os {
  using v8::Local;
  using v8::Object;
  using v8::Value;

  void InitAll(Local<Object> exports) {
    OS::Init(exports);
  }
}
}

extern "C" {
  void* _register_os() {
    return (void*)dv8::os::InitAll;
  }
}
